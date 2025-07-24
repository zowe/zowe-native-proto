#!/bin/bash
# Usage: ./generate-certs.sh [zosmfProfile] [datasetHlq] [certName]
set -e

function zowe() {
    npx -y zowe "$@" --zosmf-p $zosmfProfile
}

zosmfProfile=${1:-$(npx -y zowe config list defaults | grep "zosmf:" | awk '{print $NF}')}
datasetHlq=${2:-$(ZOWE_SHOW_SECURE_ARGS=true zowe zosmf check status --show-inputs-only | grep "user:" | awk '{print $NF}')}
certName=${3:-$zosmfProfile}
certLabel=ROOTSTAR
caLabels=("DigiCert CA" "DigiCert Global Root CA")
keyPassword=password
opensslBin=${OPENSSL_BIN:-openssl}

# Export server cert and CA certs to data sets
zowe tso issue cmd "RACDCERT SITE EXPORT(LABEL('$certLabel')) DSN('$datasetHlq.APIML.CERT.P12') FORMAT(PKCS12DER) PASSWORD('$keyPassword')"
i=1
for caLabel in "${caLabels[@]}"; do
    zowe tso issue cmd "RACDCERT CERTAUTH EXPORT(LABEL('$caLabel')) DSN('$datasetHlq.APIML.CERT.CA$i') FORMAT(CERTB64)"
    i=$((i+1))
done

# Download P12 and extract the certificate and key
zowe files download ds "$datasetHlq.APIML.CERT.P12" -f $certName.tmp.p12 --binary
$opensslBin pkcs12 -in $certName.tmp.p12 -clcerts -nokeys -password pass:$keyPassword -out - | awk '/-----BEGIN/{a=1}/-----END/{print;a=0}a' > $certName.keystore.cer
$opensslBin pkcs12 -in $certName.tmp.p12 -nocerts -nodes -password pass:$keyPassword -out - | awk '/-----BEGIN/{a=1}/-----END/{print;a=0}a' > $certName.keystore.key

# Download CA certs and concatenate to a PEM file
:> $certName.pem
i=1
for caLabel in "${caLabels[@]}"; do
    zowe files view ds "$datasetHlq.APIML.CERT.CA$i" | awk '/-----BEGIN/{a=1}/-----END/{print;a=0}a' >> $certName.pem
    i=$((i+1))
done

# Delete temporary data sets and files
zowe files delete ds "$datasetHlq.APIML.CERT.P12" -f
i=1
for caLabel in "${caLabels[@]}"; do
    zowe files delete ds "$datasetHlq.APIML.CERT.CA$i" -f
    i=$((i+1))
done
rm $certName.tmp.p12
