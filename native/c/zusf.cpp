/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#ifndef _LARGE_TIME_API
#define _LARGE_TIME_API
#endif
#ifndef _OPEN_SYS_FILE_EXT
#define _OPEN_SYS_FILE_EXT 1
#endif
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <iconv.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <map>
#include "zusf.hpp"
#include "zdyn.h"
#include "zusftype.h"
#include "zut.hpp"
#include "extern/zb64.h"
#include "iefzb4d2.h"
#ifndef _XPLATFORM_SOURCE
#define _XPLATFORM_SOURCE
#endif
#include <sys/xattr.h>
#include <time.h>
#include <iomanip>
#include <sstream>

using namespace std;

/**
 * Formats a file timestamp in ls-style format (e.g., "May 22 17:23").
 *
 * @param mtime the modification time from stat
 * @return formatted time string
 */
string zusf_format_ls_time(time_t mtime)
{
  char time_buf[32] = {0};
  struct tm *tm_info = localtime(&mtime);

  if (tm_info != nullptr)
  {
    // Format: "MMM DD HH:MM" (e.g., "May 22 17:23")
    strftime(time_buf, sizeof(time_buf), "%b %e %H:%M", tm_info);
  }
  else
  {
    strcpy(time_buf, "            "); // Fallback if time conversion fails
  }

  return string(time_buf);
}

/**
 * Gets the CCSID of a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_get_file_ccsid(ZUSF *zusf, string file)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  if (S_ISDIR(file_stats.st_mode))
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is a directory", file.c_str());
    return RTNCD_FAILURE;
  }

  return file_stats.st_tag.ft_ccsid;
}

// CCSID -> display name conversion table (based on output from `iconv -l`)
static map<int, string> create_ccsid_display_table()
{
  map<int, string> table;

  table[37] = "IBM-037";
  table[256] = "00256";
  table[259] = "00259";
  table[273] = "IBM-273";
  table[274] = "IBM-274";
  table[275] = "IBM-275";
  table[277] = "IBM-277";
  table[278] = "IBM-278";
  table[280] = "IBM-280";
  table[281] = "IBM-281";
  table[282] = "IBM-282";
  table[284] = "IBM-284";
  table[285] = "IBM-285";
  table[286] = "00286";
  table[290] = "IBM-290";
  table[293] = "00293";
  table[297] = "IBM-297";
  table[300] = "IBM-300";
  table[301] = "IBM-301";
  table[367] = "00367";
  table[420] = "IBM-420";
  table[421] = "00421";
  table[423] = "00423";
  table[424] = "IBM-424";
  table[425] = "IBM-425";
  table[437] = "IBM-437";
  table[500] = "IBM-500";
  table[720] = "00720";
  table[737] = "00737";
  table[775] = "00775";
  table[803] = "00803";
  table[806] = "00806";
  table[808] = "IBM-808";
  table[813] = "ISO8859-7";
  table[819] = "ISO8859-1";
  table[833] = "IBM-833";
  table[834] = "IBM-834";
  table[835] = "IBM-835";
  table[836] = "IBM-836";
  table[837] = "IBM-837";
  table[838] = "IBM-838";
  table[848] = "IBM-848";
  table[849] = "00849";
  table[850] = "IBM-850";
  table[851] = "00851";
  table[852] = "IBM-852";
  table[853] = "00853";
  table[855] = "IBM-855";
  table[856] = "IBM-856";
  table[857] = "00857";
  table[858] = "IBM-858";
  table[859] = "IBM-859";
  table[860] = "00860";
  table[861] = "IBM-861";
  table[862] = "IBM-862";
  table[863] = "00863";
  table[864] = "IBM-864";
  table[865] = "00865";
  table[866] = "IBM-866";
  table[867] = "IBM-867";
  table[868] = "00868";
  table[869] = "IBM-869";
  table[870] = "IBM-870";
  table[871] = "IBM-871";
  table[872] = "IBM-872";
  table[874] = "TIS-620";
  table[875] = "IBM-875";
  table[876] = "00876";
  table[878] = "00878";
  table[880] = "IBM-880";
  table[891] = "00891";
  table[895] = "00895";
  table[896] = "00896";
  table[897] = "00897";
  table[899] = "00899";
  table[901] = "IBM-901";
  table[902] = "IBM-902";
  table[903] = "00903";
  table[904] = "IBM-904";
  table[905] = "00905";
  table[912] = "ISO8859-2";
  table[913] = "00913";
  table[914] = "ISO8859-4";
  table[915] = "ISO8859-5";
  table[916] = "ISO8859-8";
  table[918] = "00918";
  table[920] = "ISO8859-9";
  table[921] = "ISO8859-13";
  table[922] = "IBM-922";
  table[923] = "ISO8859-15";
  table[924] = "IBM-924";
  table[926] = "00926";
  table[927] = "IBM-927";
  table[928] = "IBM-928";
  table[930] = "IBM-930";
  table[931] = "00931";
  table[932] = "IBM-eucJC";
  table[933] = "IBM-933";
  table[934] = "00934";
  table[935] = "IBM-935";
  table[936] = "IBM-936";
  table[937] = "IBM-937";
  table[938] = "IBM-938";
  table[939] = "IBM-939";
  table[941] = "00941";
  table[942] = "IBM-942";
  table[943] = "IBM-943";
  table[944] = "00944";
  table[946] = "IBM-946";
  table[947] = "IBM-947";
  table[948] = "IBM-948";
  table[949] = "IBM-949";
  table[950] = "BIG5";
  table[951] = "IBM-951";
  table[952] = "00952";
  table[953] = "00953";
  table[954] = "00954";
  table[955] = "00955";
  table[956] = "IBM-956";
  table[957] = "IBM-957";
  table[958] = "IBM-958";
  table[959] = "IBM-959";
  table[960] = "00960";
  table[961] = "00961";
  table[963] = "00963";
  table[964] = "IBM-eucTW";
  table[965] = "00965";
  table[966] = "00966";
  table[970] = "IBM-eucKR";
  table[971] = "00971";
  table[1002] = "01002";
  table[1004] = "01004";
  table[1006] = "01006";
  table[1008] = "01008";
  table[1009] = "01009";
  table[1010] = "01010";
  table[1011] = "01011";
  table[1012] = "01012";
  table[1013] = "01013";
  table[1014] = "01014";
  table[1015] = "01015";
  table[1016] = "01016";
  table[1017] = "01017";
  table[1018] = "01018";
  table[1019] = "01019";
  table[1020] = "01020";
  table[1021] = "01021";
  table[1023] = "01023";
  table[1025] = "IBM-1025";
  table[1026] = "IBM-1026";
  table[1027] = "IBM-1027";
  table[1040] = "01040";
  table[1041] = "01041";
  table[1042] = "01042";
  table[1043] = "01043";
  table[1046] = "IBM-1046";
  table[1047] = "IBM-1047";
  table[1051] = "01051";
  table[1088] = "IBM-1088";
  table[1089] = "ISO8859-6";
  table[1097] = "01097";
  table[1098] = "01098";
  table[1100] = "01100";
  table[1101] = "01101";
  table[1102] = "01102";
  table[1103] = "01103";
  table[1104] = "01104";
  table[1105] = "01105";
  table[1106] = "01106";
  table[1107] = "01107";
  table[1112] = "IBM-1112";
  table[1114] = "01114";
  table[1115] = "IBM-1115";
  table[1122] = "IBM-1122";
  table[1123] = "IBM-1123";
  table[1124] = "IBM-1124";
  table[1125] = "IBM-1125";
  table[1126] = "IBM-1126";
  table[1129] = "01129";
  table[1130] = "01130";
  table[1131] = "01131";
  table[1132] = "01132";
  table[1133] = "01133";
  table[1137] = "01137";
  table[1140] = "IBM-1140";
  table[1141] = "IBM-1141";
  table[1142] = "IBM-1142";
  table[1143] = "IBM-1143";
  table[1144] = "IBM-1144";
  table[1145] = "IBM-1145";
  table[1146] = "IBM-1146";
  table[1147] = "IBM-1147";
  table[1148] = "IBM-1148";
  table[1149] = "IBM-1149";
  table[1153] = "IBM-1153";
  table[1154] = "IBM-1154";
  table[1155] = "IBM-1155";
  table[1156] = "IBM-1156";
  table[1157] = "IBM-1157";
  table[1158] = "IBM-1158";
  table[1159] = "IBM-1159";
  table[1160] = "IBM-1160";
  table[1161] = "IBM-1161";
  table[1162] = "01162";
  table[1163] = "01163";
  table[1164] = "01164";
  table[1165] = "IBM-1165";
  table[1166] = "01166";
  table[1167] = "01167";
  table[1168] = "01168";
  table[1200] = "01200";
  table[1202] = "01202";
  table[1208] = "UTF-8";
  table[1210] = "01210";
  table[1232] = "01232";
  table[1250] = "IBM-1250";
  table[1251] = "IBM-1251";
  table[1252] = "IBM-1252";
  table[1253] = "IBM-1253";
  table[1254] = "IBM-1254";
  table[1255] = "IBM-1255";
  table[1256] = "IBM-1256";
  table[1257] = "01257";
  table[1258] = "01258";
  table[1275] = "01275";
  table[1276] = "01276";
  table[1277] = "01277";
  table[1280] = "01280";
  table[1281] = "01281";
  table[1282] = "01282";
  table[1283] = "01283";
  table[1284] = "01284";
  table[1285] = "01285";
  table[1287] = "01287";
  table[1288] = "01288";
  table[1350] = "01350";
  table[1351] = "01351";
  table[1362] = "IBM-1362";
  table[1363] = "IBM-1363";
  table[1364] = "IBM-1364";
  table[1370] = "IBM-1370";
  table[1371] = "IBM-1371";
  table[1374] = "01374";
  table[1375] = "01375";
  table[1380] = "IBM-1380";
  table[1381] = "IBM-1381";
  table[1382] = "01382";
  table[1383] = "IBM-eucCN";
  table[1385] = "01385";
  table[1386] = "IBM-1386";
  table[1388] = "IBM-1388";
  table[1390] = "IBM-1390";
  table[1391] = "01391";
  table[1392] = "01392";
  table[1399] = "IBM-1399";
  table[4133] = "04133";
  table[4369] = "04369";
  table[4370] = "04370";
  table[4371] = "04371";
  table[4373] = "04373";
  table[4374] = "04374";
  table[4376] = "04376";
  table[4378] = "04378";
  table[4380] = "04380";
  table[4381] = "04381";
  table[4386] = "04386";
  table[4393] = "04393";
  table[4396] = "IBM-4396";
  table[4397] = "04397";
  table[4516] = "04516";
  table[4517] = "04517";
  table[4519] = "04519";
  table[4520] = "04520";
  table[4533] = "04533";
  table[4596] = "04596";
  table[4899] = "04899";
  table[4904] = "04904";
  table[4909] = "IBM-4909";
  table[4929] = "04929";
  table[4930] = "IBM-4930";
  table[4931] = "04931";
  table[4932] = "04932";
  table[4933] = "IBM-4933";
  table[4934] = "04934";
  table[4944] = "04944";
  table[4945] = "04945";
  table[4946] = "IBM-4946";
  table[4947] = "04947";
  table[4948] = "04948";
  table[4949] = "04949";
  table[4951] = "04951";
  table[4952] = "04952";
  table[4953] = "04953";
  table[4954] = "04954";
  table[4955] = "04955";
  table[4956] = "04956";
  table[4957] = "04957";
  table[4958] = "04958";
  table[4959] = "04959";
  table[4960] = "04960";
  table[4961] = "04961";
  table[4962] = "04962";
  table[4963] = "04963";
  table[4964] = "04964";
  table[4965] = "04965";
  table[4966] = "04966";
  table[4967] = "04967";
  table[4970] = "04970";
  table[4971] = "IBM-4971";
  table[4976] = "04976";
  table[4992] = "04992";
  table[4993] = "04993";
  table[5012] = "05012";
  table[5014] = "05014";
  table[5023] = "05023";
  table[5026] = "IBM-5026";
  table[5028] = "05028";
  table[5029] = "05029";
  table[5031] = "IBM-5031";
  table[5033] = "05033";
  table[5035] = "IBM-5035";
  table[5038] = "05038";
  table[5039] = "05039";
  table[5043] = "05043";
  table[5045] = "05045";
  table[5046] = "05046";
  table[5047] = "05047";
  table[5048] = "05048";
  table[5049] = "05049";
  table[5050] = "05050";
  table[5052] = "ISO-2022-JP";
  table[5053] = "IBM-5053";
  table[5054] = "IBM-5054";
  table[5055] = "IBM-5055";
  table[5056] = "05056";
  table[5067] = "05067";
  table[5100] = "05100";
  table[5104] = "05104";
  table[5123] = "IBM-5123";
  table[5137] = "05137";
  table[5142] = "05142";
  table[5143] = "05143";
  table[5210] = "05210";
  table[5211] = "05211";
  table[5346] = "IBM-5346";
  table[5347] = "IBM-5347";
  table[5348] = "IBM-5348";
  table[5349] = "IBM-5349";
  table[5350] = "IBM-5350";
  table[5351] = "IBM-5351";
  table[5352] = "IBM-5352";
  table[5353] = "05353";
  table[5354] = "05354";
  table[5470] = "05470";
  table[5471] = "05471";
  table[5472] = "05472";
  table[5473] = "05473";
  table[5476] = "05476";
  table[5477] = "05477";
  table[5478] = "05478";
  table[5479] = "05479";
  table[5486] = "05486";
  table[5487] = "05487";
  table[5488] = "IBM-5488";
  table[5495] = "05495";
  table[8229] = "08229";
  table[8448] = "08448";
  table[8482] = "IBM-8482";
  table[8492] = "08492";
  table[8493] = "08493";
  table[8612] = "08612";
  table[8629] = "08629";
  table[8692] = "08692";
  table[9025] = "09025";
  table[9026] = "09026";
  table[9027] = "IBM-9027";
  table[9028] = "09028";
  table[9030] = "09030";
  table[9042] = "09042";
  table[9044] = "IBM-9044";
  table[9047] = "09047";
  table[9048] = "09048";
  table[9049] = "09049";
  table[9056] = "09056";
  table[9060] = "09060";
  table[9061] = "IBM-9061";
  table[9064] = "09064";
  table[9066] = "09066";
  table[9088] = "09088";
  table[9089] = "09089";
  table[9122] = "09122";
  table[9124] = "09124";
  table[9125] = "09125";
  table[9127] = "09127";
  table[9131] = "09131";
  table[9139] = "09139";
  table[9142] = "09142";
  table[9144] = "09144";
  table[9145] = "09145";
  table[9146] = "09146";
  table[9163] = "09163";
  table[9238] = "IBM-9238";
  table[9306] = "09306";
  table[9444] = "09444";
  table[9447] = "09447";
  table[9448] = "09448";
  table[9449] = "09449";
  table[9572] = "09572";
  table[9574] = "09574";
  table[9575] = "09575";
  table[9577] = "09577";
  table[9580] = "09580";
  table[12544] = "12544";
  table[12588] = "12588";
  table[12712] = "IBM-12712";
  table[12725] = "12725";
  table[12788] = "12788";
  table[13121] = "IBM-13121";
  table[13124] = "IBM-13124";
  table[13125] = "13125";
  table[13140] = "13140";
  table[13143] = "13143";
  table[13145] = "13145";
  table[13152] = "13152";
  table[13156] = "13156";
  table[13157] = "13157";
  table[13162] = "13162";
  table[13184] = "13184";
  table[13185] = "13185";
  table[13218] = "13218";
  table[13219] = "13219";
  table[13221] = "13221";
  table[13223] = "13223";
  table[13235] = "13235";
  table[13238] = "13238";
  table[13240] = "13240";
  table[13241] = "13241";
  table[13242] = "13242";
  table[13488] = "UCS-2";
  table[13671] = "13671";
  table[13676] = "13676";
  table[16421] = "16421";
  table[16684] = "IBM-16684";
  table[16804] = "IBM-16804";
  table[16821] = "16821";
  table[16884] = "16884";
  table[17221] = "17221";
  table[17240] = "17240";
  table[17248] = "IBM-17248";
  table[17314] = "17314";
  table[17331] = "17331";
  table[17337] = "17337";
  table[17354] = "17354";
  table[17584] = "17584";
  table[20517] = "20517";
  table[20780] = "20780";
  table[20917] = "20917";
  table[20980] = "20980";
  table[21314] = "21314";
  table[21317] = "21317";
  table[21344] = "21344";
  table[21427] = "21427";
  table[21433] = "21433";
  table[21450] = "21450";
  table[21680] = "21680";
  table[24613] = "24613";
  table[24876] = "24876";
  table[24877] = "24877";
  table[25013] = "25013";
  table[25076] = "25076";
  table[25426] = "25426";
  table[25427] = "25427";
  table[25428] = "25428";
  table[25429] = "25429";
  table[25431] = "25431";
  table[25432] = "25432";
  table[25433] = "25433";
  table[25436] = "25436";
  table[25437] = "25437";
  table[25438] = "25438";
  table[25439] = "25439";
  table[25440] = "25440";
  table[25441] = "25441";
  table[25442] = "25442";
  table[25444] = "25444";
  table[25445] = "25445";
  table[25450] = "25450";
  table[25467] = "25467";
  table[25473] = "25473";
  table[25479] = "25479";
  table[25480] = "25480";
  table[25502] = "25502";
  table[25503] = "25503";
  table[25504] = "25504";
  table[25508] = "25508";
  table[25510] = "25510";
  table[25512] = "25512";
  table[25514] = "25514";
  table[25518] = "25518";
  table[25520] = "25520";
  table[25522] = "25522";
  table[25524] = "25524";
  table[25525] = "25525";
  table[25527] = "25527";
  table[25546] = "25546";
  table[25580] = "25580";
  table[25616] = "25616";
  table[25617] = "25617";
  table[25618] = "25618";
  table[25619] = "25619";
  table[25664] = "25664";
  table[25690] = "25690";
  table[25691] = "25691";
  table[28709] = "IBM-28709";
  table[29109] = "29109";
  table[29172] = "29172";
  table[29522] = "29522";
  table[29523] = "29523";
  table[29524] = "29524";
  table[29525] = "29525";
  table[29527] = "29527";
  table[29528] = "29528";
  table[29529] = "29529";
  table[29532] = "29532";
  table[29533] = "29533";
  table[29534] = "29534";
  table[29535] = "29535";
  table[29536] = "29536";
  table[29537] = "29537";
  table[29540] = "29540";
  table[29541] = "29541";
  table[29546] = "29546";
  table[29614] = "29614";
  table[29616] = "29616";
  table[29618] = "29618";
  table[29620] = "29620";
  table[29621] = "29621";
  table[29623] = "29623";
  table[29712] = "29712";
  table[29713] = "29713";
  table[29714] = "29714";
  table[29715] = "29715";
  table[29760] = "29760";
  table[32805] = "32805";
  table[33058] = "33058";
  table[33205] = "33205";
  table[33268] = "33268";
  table[33618] = "33618";
  table[33619] = "33619";
  table[33620] = "33620";
  table[33621] = "33621";
  table[33623] = "33623";
  table[33624] = "33624";
  table[33632] = "33632";
  table[33636] = "33636";
  table[33637] = "33637";
  table[33665] = "33665";
  table[33698] = "33698";
  table[33699] = "33699";
  table[33700] = "33700";
  table[33717] = "33717";
  table[33722] = "EUCJP";
  table[37301] = "37301";
  table[37719] = "37719";
  table[37728] = "37728";
  table[37732] = "37732";
  table[37761] = "37761";
  table[37813] = "37813";
  table[41397] = "41397";
  table[41460] = "41460";
  table[41824] = "41824";
  table[41828] = "41828";
  table[45493] = "45493";
  table[45556] = "45556";
  table[45920] = "45920";
  table[49589] = "49589";
  table[49652] = "49652";
  table[53668] = "IBM-53668";
  table[53685] = "53685";
  table[53748] = "53748";
  table[54189] = "54189";
  table[54191] = "IBM-54191";
  table[54289] = "54289";
  table[61696] = "61696";
  table[61697] = "61697";
  table[61698] = "61698";
  table[61699] = "61699";
  table[61700] = "61700";
  table[61710] = "61710";
  table[61711] = "61711";
  table[61712] = "61712";
  table[61953] = "61953";
  table[61956] = "61956";
  table[62337] = "62337";
  table[62381] = "62381";
  table[62383] = "IBM-62383";
  table[65535] = "binary";

  return table;
}

/**
 * Gets the display name for a CCSID.
 * @param ccsid the CCSID value
 * @return display name string for the CCSID, or the CCSID number as a string if not found
 */
string zusf_get_ccsid_display_name(int ccsid)
{
  static const map<int, string> CCSID_DISPLAY_TABLE = create_ccsid_display_table();

  // Special case for invalid/unset CCSID
  if (ccsid <= 0)
  {
    return "untagged";
  }

  // O(log n) lookup in map - ideally we could use an unordered_map (hash table) for constant-time access,
  // but its still in the TR1 namespace for xlc ._.
  const auto it = CCSID_DISPLAY_TABLE.find(ccsid);
  if (it != CCSID_DISPLAY_TABLE.end())
  {
    return it->second;
  }

  // If not found in table, return the CCSID number as a string
  return zut_int_to_string(ccsid);
}

/**
 * Builds a file mode string from stat mode.
 *
 * @param mode the file mode from stat
 *
 * @return mode string in the format "drwxrwxrwx"
 */
string zusf_build_mode_string(mode_t mode)
{
  string mode_str;
  mode_str += (S_ISDIR(mode) ? "d" : "-");
  mode_str += (mode & S_IRUSR ? "r" : "-");
  mode_str += (mode & S_IWUSR ? "w" : "-");
  mode_str += (mode & S_IXUSR ? "x" : "-");
  mode_str += (mode & S_IRGRP ? "r" : "-");
  mode_str += (mode & S_IWGRP ? "w" : "-");
  mode_str += (mode & S_IXGRP ? "x" : "-");
  mode_str += (mode & S_IROTH ? "r" : "-");
  mode_str += (mode & S_IWOTH ? "w" : "-");
  mode_str += (mode & S_IXOTH ? "x" : "-");
  return mode_str;
}

/**
 * Creates a USS file or directory.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param mode mode of the file or directory
 * @param createDir flag indicating whether to create a directory
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_create_uss_file_or_dir(ZUSF *zusf, string file, mode_t mode, bool createDir)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) != -1)
  {
    if (S_ISREG(file_stats.st_mode))
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "File '%s' already exists", file.c_str());
    }
    else if (S_ISDIR(file_stats.st_mode))
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Directory '%s' already exists", file.c_str());
    }
    else
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' already exists! Mode: '%08o'", file.c_str(), file_stats.st_mode);
    }
    return RTNCD_FAILURE;
  }

  if (createDir)
  {
    const auto last_trailing_slash = file.find_last_of("/");
    if (last_trailing_slash != std::string::npos)
    {
      const auto parent_path = file.substr(0, last_trailing_slash);
      const auto exists = stat(parent_path.c_str(), &file_stats) == 0;
      if (!exists)
      {
        const auto rc = zusf_create_uss_file_or_dir(zusf, parent_path, mode, true);
        if (rc != 0)
        {
          return rc;
        }
      }
    }
    const auto rc = mkdir(file.c_str(), mode);
    if (rc != 0)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to create directory '%s', errno: %d", file.c_str(), errno);
    }
    return rc;
  }
  else
  {
    ofstream out(file.c_str());
    if (out.is_open())
    {
      out.close();
      chmod(file.c_str(), mode);
      return RTNCD_SUCCESS;
    }
  }

  zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not create '%s'", file.c_str());
  return RTNCD_FAILURE;
}

/**
 * Formats file information for listing output.
 *
 * @param zusf pointer to a ZUSF object
 * @param file_stats stat structure for the file
 * @param file_path full path to the file (for CCSID lookup)
 * @param display_name name to display in the listing
 * @param options listing options (all_files, long_format)
 * @param use_csv_format whether to use CSV format or ls-style format
 *
 * @return formatted string for the file entry
 */
string zusf_format_file_entry(ZUSF *zusf, const struct stat &file_stats, const string &file_path, const string &display_name, ListOptions options, bool use_csv_format)
{
  if (!options.long_format)
  {
    return display_name + "\n";
  }

  const string mode = zusf_build_mode_string(file_stats.st_mode);
  const auto ccsid = zusf_get_ccsid_display_name(file_stats.st_tag.ft_ccsid);
  const auto tag_flag = (file_stats.st_tag.ft_txtflag) ? "T=on" : "T=off";
  const string time_str = zusf_format_ls_time(file_stats.st_mtime);

  if (use_csv_format)
  {
    vector<string> fields;
    fields.push_back(mode);
    fields.push_back(zut_int_to_string(file_stats.st_nlink));
    fields.push_back(zusf_get_owner_from_uid(file_stats.st_uid));
    fields.push_back(zusf_get_group_from_gid(file_stats.st_gid));
    fields.push_back(zut_int_to_string(file_stats.st_size));
    fields.push_back(ccsid);
    fields.push_back(time_str);
    fields.push_back(display_name);
    return zut_format_as_csv(fields) + "\n";
  }
  else
  {
    // ls-style format: "- untagged    T=off -rw-r--r--   1 TRAE     GRPOMVS  2772036 May 22 17:23 hw.txt"
    stringstream ss;
    const auto tagged = ccsid != "untagged";
    const auto tag_prefix = tagged ? "t" : "-";
    ss << tag_prefix << " " << left << setw(12) << ccsid
       << " " << setw(5) << tag_flag
       << " " << mode
       << " " << right << setw(3) << file_stats.st_nlink
       << " " << left << setw(8) << zusf_get_owner_from_uid(file_stats.st_uid)
       << " " << setw(8) << zusf_get_group_from_gid(file_stats.st_gid)
       << " " << right << setw(8) << file_stats.st_size
       << " " << time_str
       << " " << display_name << "\n";
    return ss.str();
  }
}

/**
 * Lists the USS file path.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file or directory
 * @param response reference to a string where the read data will be stored
 * @param options listing options (all_files, long_format)
 * @param use_csv_format whether to use CSV format or ls-style format
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_list_uss_file_path(ZUSF *zusf, string file, string &response, ListOptions options, bool use_csv_format)
{
  // TODO(zFernand0): Handle `*` and other bash-expansion rules
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  // TODO: Hide hidden paths by default
  // TODO(zFernand0): Add option to list full file paths
  // TODO(zFernand0): Add option to list file tags

  if (S_ISREG(file_stats.st_mode))
  {
    const auto file_name = file.substr(file.find_last_of("/") + 1);
    response = zusf_format_file_entry(zusf, file_stats, file, file_name, options, use_csv_format);
    return RTNCD_SUCCESS;
  }

  if (!S_ISDIR(file_stats.st_mode))
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is not a directory", file.c_str());
    return RTNCD_FAILURE;
  }

  DIR *dir;
  if ((dir = opendir(file.c_str())) == nullptr)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  // Collect all directory entries first
  vector<string> entry_names;
  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr)
  {
    if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
    {
      string name = entry->d_name;
      // Skip hidden files if not requested
      if (name.at(0) == '.' && !options.all_files)
      {
        continue;
      }
      entry_names.push_back(name);
    }
  }
  closedir(dir);

  // Sort entries alphabetically
  sort(entry_names.begin(), entry_names.end());

  // Process sorted entries
  response.clear();
  for (auto i = 0u; i < entry_names.size(); i++)
  {
    const auto name = entry_names.at(i);
    string child_path = file[file.length() - 1] == '/' ? file + name : file + "/" + name;
    struct stat child_stats;
    stat(child_path.c_str(), &child_stats);

    response += zusf_format_file_entry(zusf, child_stats, child_path, name, options, use_csv_format);
  }

  return RTNCD_SUCCESS;
}

/**
 * Reads data from a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param response reference to a string where the read data will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_read_from_uss_file(ZUSF *zusf, string file, string &response)
{
  const auto bpxk_autocvt = getenv("_BPXK_AUTOCVT");
  setenv("_BPXK_AUTOCVT", "OFF", 1);

  FILE *fp = fopen(file.c_str(), "rb");
  if (!fp)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open file '%s'", file.c_str());
    setenv("_BPXK_AUTOCVT", bpxk_autocvt, 1);
    return RTNCD_FAILURE;
  }

  size_t bytes_read = 0;
  size_t total_size = 0;
  char buffer[4096] = {0};
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
  {
    total_size += bytes_read;
    response.append(buffer, bytes_read);
  }
  fclose(fp);

  // Use file tag encoding if available, otherwise fall back to provided encoding
  string encoding_to_use;
  bool has_encoding = false;

  if (zusf->encoding_opts.data_type == eDataTypeText && strlen(zusf->encoding_opts.codepage) > 0)
  {
    encoding_to_use = string(zusf->encoding_opts.codepage);
    has_encoding = true;
  }
  else
  {
    // Try to get the file's CCSID first
    int file_ccsid = zusf_get_file_ccsid(zusf, file);
    if (file_ccsid > 0 && file_ccsid != 65535) // Valid CCSID and not binary
    {
      encoding_to_use = zusf_get_ccsid_display_name(file_ccsid);
      has_encoding = true;
    }
  }

  cout << "encoding_to_use: " << encoding_to_use << endl;
  cout << "size: " << total_size << endl;
  cout << "response before encoding: " << endl;
  zut_print_string_as_bytes(response);

  if (total_size > 0 && has_encoding)
  {
    try
    {
      response = zut_encode(response, encoding_to_use, "UTF-8", zusf->diag);
    }
    catch (std::exception &e)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from %s to UTF-8", encoding_to_use.c_str());
      setenv("_BPXK_AUTOCVT", bpxk_autocvt, 1);
      return RTNCD_FAILURE;
    }
  }

  setenv("_BPXK_AUTOCVT", bpxk_autocvt, 1);
  return RTNCD_SUCCESS;
}

/**
 * Reads data from a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param pipe name of the output pipe
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_read_from_uss_file_streamed(ZUSF *zusf, string file, string pipe)
{
  FILE *fin = fopen(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? "rb" : "r");
  if (!fin)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open file '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  int fifo_fd = open(pipe.c_str(), O_WRONLY);
  FILE *fout = fdopen(fifo_fd, "w");
  if (!fout)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open output pipe '%s'", pipe.c_str());
    return RTNCD_FAILURE;
  }

  // Use file tag encoding if available, otherwise fall back to provided encoding
  string encoding_to_use;
  bool has_encoding = false;

  if (zusf->encoding_opts.data_type == eDataTypeText)
  {
    // Try to get the file's CCSID first
    int file_ccsid = zusf_get_file_ccsid(zusf, file);
    if (file_ccsid > 0 && file_ccsid != 65535) // Valid CCSID, not binary
    {
      encoding_to_use = zut_int_to_string(file_ccsid);
      has_encoding = true;
    }
    else if (strlen(zusf->encoding_opts.codepage) > 0)
    {
      encoding_to_use = string(zusf->encoding_opts.codepage);
      has_encoding = true;
    }
  }

  const size_t chunk_size = FIFO_CHUNK_SIZE * 3 / 4;
  std::vector<char> buf(chunk_size);
  size_t bytes_read;

  while ((bytes_read = fread(&buf[0], 1, chunk_size, fin)) > 0)
  {
    int chunk_len = bytes_read;
    const char *chunk = &buf[0];
    std::vector<char> temp_encoded;

    if (has_encoding)
    {
      try
      {
        temp_encoded = zut_encode(chunk, chunk_len, encoding_to_use, "UTF-8", zusf->diag);
        chunk = &temp_encoded[0];
        chunk_len = temp_encoded.size();
      }
      catch (std::exception &e)
      {
        zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from %s to UTF-8", encoding_to_use.c_str());
        return RTNCD_FAILURE;
      }
    }

    chunk = base64(chunk, chunk_len, &chunk_len);
    fwrite(chunk, 1, chunk_len, fout);
  }

  fflush(fout);
  fclose(fin);
  fclose(fout);

  return RTNCD_SUCCESS;
}

/**
 * Writes data to a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param data string to be written to the file
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_write_to_uss_file(ZUSF *zusf, string file, string &data)
{
  // TODO(zFernand0): Avoid overriding existing files
  struct stat file_stats;

  int stat_result = stat(file.c_str(), &file_stats);
  if (strlen(zusf->etag) > 0 && stat_result != -1)
  {
    const auto current_etag = zut_build_etag(file_stats.st_mtime, file_stats.st_size);
    if (current_etag != zusf->etag)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Etag mismatch: expected %s, actual %s", zusf->etag, current_etag.c_str());
      return RTNCD_FAILURE;
    }
  }
  if (stat_result == -1 && errno != ENOENT)
    return RTNCD_FAILURE;
  zusf->created = stat_result == -1;

  // Use file tag encoding if available, otherwise fall back to provided encoding
  string encoding_to_use;
  bool has_encoding = false;

  if (zusf->encoding_opts.data_type == eDataTypeText)
  {
    // Try to get the file's CCSID first
    int file_ccsid = zusf_get_file_ccsid(zusf, file);
    if (file_ccsid > 0 && file_ccsid != 65535) // Valid CCSID and not binary
    {
      encoding_to_use = zut_int_to_string(file_ccsid);
      has_encoding = true;
    }
    else if (strlen(zusf->encoding_opts.codepage) > 0)
    {
      encoding_to_use = string(zusf->encoding_opts.codepage);
      has_encoding = true;
    }
  }

  std::string temp = data;
  if (has_encoding)
  {
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, "UTF-8", encoding_to_use, zusf->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from UTF-8 to %s", encoding_to_use.c_str());
      return RTNCD_FAILURE;
    }
  }
  const char *mode = (zusf->encoding_opts.data_type == eDataTypeBinary) ? "wb" : "w";
  FILE *fp = std::fopen(file.c_str(), mode);
  if (!fp)
  {
    zusf->diag.e_msg_len = std::sprintf(zusf->diag.e_msg, "Could not open '%s' for writing", file.c_str());
    return RTNCD_FAILURE;
  }

  if (!temp.empty())
    std::fwrite(temp.data(), 1, temp.size(), fp);
  std::fclose(fp);

  struct stat new_stats;
  if (stat(file.c_str(), &new_stats) == -1)
  {
    zusf->diag.e_msg_len = std::sprintf(
        zusf->diag.e_msg,
        "Could not stat file '%s' after writing",
        file.c_str());
    return RTNCD_FAILURE;
  }

  const string new_tag = zut_build_etag(new_stats.st_mtime, new_stats.st_size);
  std::strcpy(zusf->etag, new_tag.c_str());

  return RTNCD_SUCCESS; // success
}

/**
 * Writes data to a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param pipe name of the input pipe
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_write_to_uss_file_streamed(ZUSF *zusf, string file, string pipe)
{
  // TODO(zFernand0): Avoid overriding existing files
  struct stat file_stats;

  // Use file tag encoding if available, otherwise fall back to provided encoding
  string encoding_to_use;
  bool has_encoding = false;

  if (zusf->encoding_opts.data_type == eDataTypeText)
  {
    // Try to get the file's CCSID first
    int file_ccsid = zusf_get_file_ccsid(zusf, file);
    if (file_ccsid > 0 && file_ccsid != 65535) // Valid CCSID and not binary
    {
      encoding_to_use = zut_int_to_string(file_ccsid);
      has_encoding = true;
    }
    else if (strlen(zusf->encoding_opts.codepage) > 0)
    {
      encoding_to_use = string(zusf->encoding_opts.codepage);
      has_encoding = true;
    }
  }

  int stat_result = stat(file.c_str(), &file_stats);
  if (strlen(zusf->etag) > 0 && stat_result != -1)
  {
    const auto current_etag = zut_build_etag(file_stats.st_mtime, file_stats.st_size);
    if (current_etag != zusf->etag)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Etag mismatch: expected %s, actual %s", zusf->etag, current_etag.c_str());
      return RTNCD_FAILURE;
    }
  }
  if (stat_result == -1 && errno != ENOENT)
    return RTNCD_FAILURE;
  zusf->created = stat_result == -1;

  FILE *fout = fopen(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? "wb" : "w");
  if (!fout)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  int fifo_fd = open(pipe.c_str(), O_RDONLY);
  FILE *fin = fdopen(fifo_fd, "r");
  if (!fin)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open input pipe '%s'", pipe.c_str());
    fclose(fout);
    return RTNCD_FAILURE;
  }

  std::vector<char> buf(FIFO_CHUNK_SIZE);
  size_t bytes_read;

  while ((bytes_read = fread(&buf[0], 1, FIFO_CHUNK_SIZE, fin)) > 0)
  {
    int chunk_len;
    const char *chunk = (char *)unbase64(&buf[0], bytes_read, &chunk_len);
    std::vector<char> temp_encoded;

    if (has_encoding)
    {
      try
      {
        temp_encoded = zut_encode(chunk, chunk_len, "UTF-8", encoding_to_use, zusf->diag);
        chunk = &temp_encoded[0];
        chunk_len = temp_encoded.size();
      }
      catch (std::exception &e)
      {
        zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from UTF-8 to %s", encoding_to_use.c_str());
        fclose(fin);
        fclose(fout);
        return RTNCD_FAILURE;
      }
    }

    size_t bytes_written = fwrite(chunk, 1, chunk_len, fout);
    if (bytes_written != chunk_len)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to write to '%s' (possibly out of space)", file.c_str());
      fclose(fin);
      fclose(fout);
      return RTNCD_FAILURE;
    }
  }

  fflush(fout);
  fclose(fin);
  fclose(fout);

  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  // Print new e-tag to stdout as response
  string etag_str = zut_build_etag(file_stats.st_mtime, file_stats.st_size);
  strcpy(zusf->etag, etag_str.c_str());

  return RTNCD_SUCCESS;
}

/**
 * Changes the permissions of a USS file or directory.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param mode new permissions in octal format
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_chmod_uss_file_or_dir(ZUSF *zusf, string file, mode_t mode, bool recursive)
{
  // TODO(zFernand0): Add recursive option for directories
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  if (!recursive && S_ISDIR(file_stats.st_mode))
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is a folder and recursive is false", file.c_str());
    return RTNCD_FAILURE;
  }

  chmod(file.c_str(), mode);
  if (recursive && S_ISDIR(file_stats.st_mode))
  {
    DIR *dir;
    if ((dir = opendir(file.c_str())) == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = file[file.length() - 1] == '/' ? file + string((const char *)entry->d_name)
                                                                 : file + string("/") + string((const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chmod_uss_file_or_dir(zusf, child_path, mode, S_ISDIR(file_stats.st_mode));
        if (rc != 0)
        {
          return rc;
        }
      }
    }
  }
  return 0;
}

int zusf_delete_uss_item(ZUSF *zusf, string file, bool recursive)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  const auto is_dir = S_ISDIR(file_stats.st_mode);
  if (is_dir && !recursive)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is a directory and recursive was false", file.c_str());
    return RTNCD_FAILURE;
  }

  if (is_dir)
  {
    DIR *dir;
    if ((dir = opendir(file.c_str())) == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = file[file.length() - 1] == '/' ? file + string((const char *)entry->d_name)
                                                                 : file + string("/") + string((const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_delete_uss_item(zusf, child_path, S_ISDIR(file_stats.st_mode));
        if (rc != 0)
        {
          return rc;
        }
      }
    }
    closedir(dir);
  }

  const auto rc = is_dir ? rmdir(file.c_str()) : remove(file.c_str());
  if (rc != 0)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not delete '%s', rc: %d", file.c_str(), errno);
    return RTNCD_FAILURE;
  }

  return 0;
}

const char *zusf_get_owner_from_uid(uid_t uid)
{
  auto *meta = getpwuid(uid);
  return meta ? meta->pw_name : nullptr;
}

const char *zusf_get_group_from_gid(gid_t gid)
{
  auto *meta = getgrgid(gid);
  return meta ? meta->gr_name : nullptr;
}

short zusf_get_id_from_user_or_group(string user_or_group, bool is_user)
{
  const auto is_numeric = user_or_group.find_first_not_of("0123456789") == std::string::npos;
  if (is_numeric)
  {
    return (short)atoi(user_or_group.c_str());
  }

  auto *meta = is_user ? (void *)getpwnam(user_or_group.c_str()) : (void *)getgrnam(user_or_group.c_str());
  if (meta)
  {
    return is_user ? ((passwd *)meta)->pw_uid : ((group *)meta)->gr_gid;
  }

  return -1;
}

int zusf_chown_uss_file_or_dir(ZUSF *zusf, string file, string owner, bool recursive)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  if (S_ISDIR(file_stats.st_mode) && !recursive)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is a folder and recursive is false", file.c_str());
    return RTNCD_FAILURE;
  }

  const auto uid = zusf_get_id_from_user_or_group(owner, true);
  const auto colon_pos = owner.find_first_of(":");
  const auto group = colon_pos != std::string::npos ? owner.substr(colon_pos + 1) : std::string();
  const auto gid = group.empty() ? file_stats.st_gid : zusf_get_id_from_user_or_group(group, false);
  const auto rc = chown(file.c_str(), uid, gid);

  if (rc != 0)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "chmod failed for path '%s', errno %d", file.c_str(), errno);
    return RTNCD_FAILURE;
  }

  if (recursive && S_ISDIR(file_stats.st_mode))
  {
    DIR *dir;
    if ((dir = opendir(file.c_str())) == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = file[file.length() - 1] == '/' ? file + string((const char *)entry->d_name)
                                                                 : file + string("/") + string((const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chown_uss_file_or_dir(zusf, child_path, owner, S_ISDIR(file_stats.st_mode));
        if (rc != 0)
        {
          return rc;
        }
      }
    }
  }

  return 0;
}

int zusf_chtag_uss_file_or_dir(ZUSF *zusf, string file, string tag, bool recursive)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  const auto ccsid = strtol(tag.c_str(), nullptr, 10);
  if (ccsid == LONG_MAX || ccsid == LONG_MIN)
  {
    // TODO(traeok): Get CCSID from encoding name
  }
  const auto is_dir = S_ISDIR(file_stats.st_mode);
  if (!is_dir)
  {
    attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.att_filetagchg = 1;
    attr.att_filetag.ft_ccsid = ccsid;
    attr.att_filetag.ft_txtflag = int(ccsid != 65535);

    const auto rc = __chattr((char *)file.c_str(), &attr, sizeof(attr));
    if (rc != 0)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to update attributes for path '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
  }
  else if (recursive)
  {
    DIR *dir;
    if ((dir = opendir(file.c_str())) == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = file[file.length() - 1] == '/' ? file + string((const char *)entry->d_name)
                                                                 : file + string("/") + string((const char *)entry->d_name);
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chtag_uss_file_or_dir(zusf, child_path, tag, S_ISDIR(file_stats.st_mode));
        if (rc != 0)
        {
          return rc;
        }
      }
    }
  }
  return 0;
}
