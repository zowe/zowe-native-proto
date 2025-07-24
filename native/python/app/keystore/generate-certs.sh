#!/bin/bash
# Usage: ./generate-certs.sh <zosmfProfile> <userId> <certName>
set -e

zosmfProfile=$1
userId=$2
certName=${3:-$zosmfProfile}
certLabel=ROOTSTAR
caLabels=("DigiCert CA" "DigiCert Global Root CA")
keyPassword=password

# Export server cert and CA certs to data sets
zowe tso issue cmd "RACDCERT SITE EXPORT(LABEL('$certLabel')) DSN('$userId.APIML.CERT.P12') FORMAT(PKCS12DER) PASSWORD('$keyPassword')" --zosmf-p $zosmfProfile
i=1
for caLabel in "${caLabels[@]}"; do
    zowe tso issue cmd "RACDCERT CERTAUTH EXPORT(LABEL('$caLabel')) DSN('$userId.APIML.CERT.CA$i') FORMAT(CERTB64)" --zosmf-p $zosmfProfile
    i=$((i+1))
done

# Download P12 and extract the certificate and key
zowe files download ds "$userId.APIML.CERT.P12" -f $certName.tmp.p12 --binary --zosmf-p $zosmfProfile
openssl pkcs12 -in $certName.tmp.p12 -clcerts -nokeys -password pass:$keyPassword -out - | awk '/-----BEGIN/{a=1}/-----END/{print;a=0}a' > $certName.keystore.cer
openssl pkcs12 -in $certName.tmp.p12 -nocerts -nodes -password pass:$keyPassword -out - | awk '/-----BEGIN/{a=1}/-----END/{print;a=0}a' > $certName.keystore.key

# Download CA certs and concatenate to a PEM file
:> $certName.pem
i=1
for caLabel in "${caLabels[@]}"; do
    zowe files view ds "$userId.APIML.CERT.CA$i" --zosmf-p $zosmfProfile | awk '/-----BEGIN/{a=1}/-----END/{print;a=0}a' >> $certName.pem
    i=$((i+1))
done

# Delete temporary data sets and files
zowe files delete ds "$userId.APIML.CERT.P12" -f --zosmf-p $zosmfProfile
i=1
for caLabel in "${caLabels[@]}"; do
    zowe files delete ds "$userId.APIML.CERT.CA$i" -f --zosmf-p $zosmfProfile
    i=$((i+1))
done
rm $certName.tmp.p12
