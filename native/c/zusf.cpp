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

// z/OS UNIX extensions needed for st_tag in struct stat, etc.
#ifndef _AE_BIMODAL
#define _AE_BIMODAL 1
#endif
#ifndef _OPEN_SYS_FILE_EXT
#define _OPEN_SYS_FILE_EXT 1
#endif
#ifndef _LARGE_TIME_API
#define _LARGE_TIME_API
#endif

#include <limits.h>
#include <limits>
#include <climits>

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
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#include <unistd.h>
#include <stdlib.h>
#include <map>
#include "zusf.hpp"
#include "zdyn.h"
#include "zusftype.h"
#include "zut.hpp"
#include "zbase64.h"
#include "iefzb4d2.h"

#ifndef _XPLATFORM_SOURCE
#define _XPLATFORM_SOURCE
#endif
#include <sys/xattr.h>
#include <vector>
#include <time.h>
#include <iomanip>
#include <sstream>
#include <errno.h>
using namespace std;

/**
 * Concatenates a directory path with a file/directory name, handling trailing slashes.
 *
 * @param dir_path the directory path
 * @param name the file or directory name to append
 * @return the concatenated path
 */
string zusf_join_path(const string &dir_path, const string &name)
{
  return dir_path[dir_path.length() - 1] == '/' ? dir_path + name : dir_path + "/" + name;
}

/**
 * Formats a file timestamp.
 *
 * @param mtime the modification time from stat
 * @param use_csv_format whether to use CSV format (ISO time in UTC) or ls-style format (local time)
 * @return formatted time string
 */
string zusf_format_ls_time(time_t mtime, bool use_csv_format)
{
  char time_buf[32] = {0};

  if (use_csv_format)
  {
    // CSV format: ISO time in UTC (2024-01-31T05:30:00Z)
    struct tm *tm_info = gmtime(&mtime);
    if (tm_info != nullptr)
    {
      strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%S", tm_info);
    }
    else
    {
      strcpy(time_buf, "1970-01-01T00:00:00"); // Fallback if time conversion fails
    }
  }
  else
  {
    // ls-style format: local time (May 22 17:23)
    struct tm *tm_info = localtime(&mtime);
    if (tm_info != nullptr)
    {
      strftime(time_buf, sizeof(time_buf), "%b %e %H:%M", tm_info);
    }
    else
    {
      strcpy(time_buf, "            "); // Fallback if time conversion fails
    }
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
 * Gets the CCSID from a display name.
 * @param display_name the display name string
 * @return CCSID value, or -1 if not found
 */
int zusf_get_ccsid_from_display_name(const string &display_name)
{
  static const map<int, string> CCSID_DISPLAY_TABLE = create_ccsid_display_table();

  // Handle special cases
  if (display_name == "untagged")
  {
    return 0;
  }
  if (display_name == "binary")
  {
    return 65535;
  }

  // Search through the table for a matching display name
  for (map<int, string>::const_iterator it = CCSID_DISPLAY_TABLE.begin();
       it != CCSID_DISPLAY_TABLE.end(); ++it)
  {
    if (it->second == display_name)
    {
      return it->first;
    }
  }

  return -1; // Not found
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

  // Determine file type character
  if (S_ISDIR(mode))
  {
    mode_str += "d"; // directory
  }
  else if (S_ISLNK(mode))
  {
    mode_str += "l"; // symbolic link
  }
  else if (S_ISFIFO(mode))
  {
    mode_str += "p"; // named pipe (FIFO)
  }
  else if (S_ISSOCK(mode))
  {
    mode_str += "s"; // socket
  }
  else if (S_ISCHR(mode))
  {
    mode_str += "c"; // character device
  }
  else
  {
    mode_str += "-"; // regular file or unknown
  }

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
 * Copies a USS file or directory.
 */
int zusf_copy_file_or_dir(ZUSF *zusf, const string &source_path, const string &destination_path, CopyOptions options) {

  string command_flags = "";
  if (options.recursive)
  {
    command_flags += "-R ";
  }
  if (options.follow_symlinks)
  {
    command_flags += "-L ";
  }
  if (options.preserve_attributes)
  {
    command_flags += "-p ";
  }
  if (options.force)
  {
    command_flags += "-f ";
  }
  string cp_command = "cp " + command_flags + " \"" + source_path + "\" \"" + destination_path + "\" 2>&1";
  string response;
  int rc = zut_run_shell_command(cp_command, response);
  if (rc > 0) {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Error: %s\n\t return code: %d", response.c_str(), rc);
    return RTNCD_FAILURE;
  }
  return rc;
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
int zusf_create_uss_file_or_dir(ZUSF *zusf, const string &file, mode_t mode, bool createDir)
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
        if (0 != rc)
        {
          return rc;
        }
      }
    }
    const auto rc = mkdir(file.c_str(), mode);
    if (0 != rc)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to create directory '%s', errno: %d", file.c_str(), errno);
    }
    chmod(file.c_str(), mode);
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
  const string time_str = zusf_format_ls_time(file_stats.st_mtime, use_csv_format);

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
    // ls-style format: "- untagged    T=off -rw-r--r--   1 TRAE     XMPLGRP  2772036 May 22 17:23 hw.txt"
    stringstream ss;
    const auto is_directory = S_ISDIR(file_stats.st_mode);
    const auto tagged = !is_directory && ccsid != "untagged";
    const auto tag_prefix = tagged ? "t" : "-";

    ss << (is_directory ? "" : tag_prefix) << "  " << left << setw(12);
    ss << (is_directory ? "" : ccsid);
    ss << " " << setw(5) << (is_directory ? "" : tag_flag)
       << (is_directory ? "   " : "  ") << mode
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
 * Recursive helper function to collect directory entries with depth control.
 *
 * @param zusf pointer to a ZUSF object
 * @param dir_path path to the directory
 * @param entry_names reference to vector where entry names will be stored
 * @param options listing options (all_files, long_format, depth)
 * @param current_depth current recursion depth
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
static int zusf_collect_directory_entries_recursive(ZUSF *zusf, const string &dir_path, vector<string> &entry_names, const ListOptions &options, int current_depth = 0)
{
  DIR *dir;
  if ((dir = opendir(dir_path.c_str())) == nullptr)
  {
    return RTNCD_FAILURE;
  }

  // Collect all directory entries first
  vector<string> current_entries;
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
      current_entries.push_back(name);
    }
  }
  closedir(dir);

  // Sort entries alphabetically using C string comparison
  sort(current_entries.begin(), current_entries.end(), zut_string_compare_c);

  // Add current level entries to the result
  for (vector<string>::const_iterator it = current_entries.begin(); it != current_entries.end(); ++it)
  {
    const string &name = *it;
    entry_names.push_back(name);

    // If we haven't reached max depth, recurse into subdirectories
    if (options.max_depth > 1 && current_depth < (options.max_depth - 1))
    {
      string child_path = zusf_join_path(dir_path, name);
      struct stat child_stats;
      // Use lstat so symlinked directories are reported as links, not traversed as directories.
      if (lstat(child_path.c_str(), &child_stats) == 0 && S_ISDIR(child_stats.st_mode))
      {
        vector<string> subdir_entries;
        if (zusf_collect_directory_entries_recursive(zusf, child_path, subdir_entries, options, current_depth + 1) == RTNCD_SUCCESS)
        {
          // Add subdirectory entries with path prefix
          for (vector<string>::const_iterator sub_it = subdir_entries.begin(); sub_it != subdir_entries.end(); ++sub_it)
          {
            const string &subentry = *sub_it;
            entry_names.push_back(name + "/" + subentry);
          }
        }
      }
    }
  }

  return RTNCD_SUCCESS;
}

/**
 * Lists the USS file path.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file or directory
 * @param response reference to a string where the read data will be stored
 * @param options listing options (all_files, long_format, max_depth)
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

  // TODO(zFernand0): Add option to list full file paths

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

  response.clear();

  // Treat depth == 0 as "ls -d" behavior: show the directory itself, not its contents
  if (options.max_depth == 0)
  {
    const auto dir_name = file.substr(file.find_last_of("/") + 1);
    response = zusf_format_file_entry(zusf, file_stats, file, dir_name, options, use_csv_format);
    return RTNCD_SUCCESS;
  }

  // Add "." and ".." entries if all_files option is set
  if (options.all_files)
  {
    // Add "." entry
    response += zusf_format_file_entry(zusf, file_stats, file, ".", options, use_csv_format);

    // Add ".." entry if we can stat the parent directory
    string parent_path = file.substr(0, file.find_last_of("/"));
    if (parent_path.empty())
    {
      parent_path = "/"; // Root directory case
    }
    struct stat parent_stats;
    if (stat(parent_path.c_str(), &parent_stats) == 0)
    {
      response += zusf_format_file_entry(zusf, parent_stats, parent_path, "..", options, use_csv_format);
    }
  }

  // Collect all directory entries (recursively if depth > 1)
  vector<string> entry_names;
  if (zusf_collect_directory_entries_recursive(zusf, file, entry_names, options) != RTNCD_SUCCESS)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  // Process sorted entries
  for (auto i = 0u; i < entry_names.size(); i++)
  {
    const auto name = entry_names.at(i);
    string child_path = zusf_join_path(file, name);
    struct stat child_stats;
    if (lstat(child_path.c_str(), &child_stats) != 0)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not stat child path '%s'", child_path.c_str());
      return RTNCD_FAILURE;
    }

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
int zusf_read_from_uss_file(ZUSF *zusf, const string &file, string &response)
{
  AutocvtGuard autocvt(false);
  ifstream in(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? ifstream::in | ifstream::binary : ifstream::in);
  if (!in.is_open())
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open file '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  in.seekg(0, ios::end);
  size_t size = in.tellg();
  in.seekg(0, ios::beg);

  vector<char> raw_data(size);
  in.read(&raw_data[0], size);

  response.assign(raw_data.begin(), raw_data.end());
  in.close();

  // Use file tag encoding if available, otherwise fall back to provided encoding
  string encoding_to_use;
  bool has_encoding = false;

  if (zusf->encoding_opts.data_type == eDataTypeText)
  {
    if (strlen(zusf->encoding_opts.codepage) > 0)
    {
      encoding_to_use = string(zusf->encoding_opts.codepage);
      has_encoding = true;
    }
    else
    {
      // Use tagged encoding if valid CCSID and not UTF-8 or binary
      int file_ccsid = zusf_get_file_ccsid(zusf, file);
      if (file_ccsid > 0 && file_ccsid != 1208 && file_ccsid != 65535)
      {
        encoding_to_use = zut_int_to_string(file_ccsid);
        has_encoding = true;
      }
    }
  }

  if (size > 0 && has_encoding)
  {
    string temp = response;
    const auto source_encoding = strlen(zusf->encoding_opts.source_codepage) > 0 ? string(zusf->encoding_opts.source_codepage) : "UTF-8";
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, encoding_to_use, source_encoding, zusf->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from %s to %s", source_encoding.c_str(), encoding_to_use.c_str());
      return RTNCD_FAILURE;
    }
    if (!temp.empty())
    {
      response = temp;
    }
  }

  return RTNCD_SUCCESS;
}

/**
 * Reads data from a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param pipe name of the output pipe
 * @param content_len pointer where the length of the file contents will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_read_from_uss_file_streamed(ZUSF *zusf, const string &file, const string &pipe, size_t *content_len)
{
  if (content_len == nullptr)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "content_len must be a valid size_t pointer");
    return RTNCD_FAILURE;
  }

  AutocvtGuard autocvt(false);
  FileGuard fin(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? "rb" : "r");
  if (!fin)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open file '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  struct stat st;
  if (stat(file.c_str(), &st) != 0)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not stat file '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

#if defined(__clang__)
  if (zusf->set_size_callback)
  {
    zusf->set_size_callback((uint64_t)st.st_size);
  }
#endif

  int fifo_fd = open(pipe.c_str(), O_WRONLY);
  if (fifo_fd == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "open() failed on output pipe '%s', errno: %d", pipe.c_str(), errno);
    return RTNCD_FAILURE;
  }

  FileGuard fout(fifo_fd, "w");
  if (!fout)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open output pipe '%s'", pipe.c_str());
    close(fifo_fd);
    return RTNCD_FAILURE;
  }

  // Use file tag encoding if available, otherwise fall back to provided encoding
  string encoding_to_use;
  bool has_encoding = false;

  if (zusf->encoding_opts.data_type == eDataTypeText)
  {
    if (strlen(zusf->encoding_opts.codepage) > 0)
    {
      encoding_to_use = string(zusf->encoding_opts.codepage);
      has_encoding = true;
    }
    else
    {
      // Use tagged encoding if valid CCSID and not UTF-8 or binary
      int file_ccsid = zusf_get_file_ccsid(zusf, file);
      if (file_ccsid > 0 && file_ccsid != 1208 && file_ccsid != 65535)
      {
        encoding_to_use = zut_int_to_string(file_ccsid);
        has_encoding = true;
      }
    }
  }

  const size_t chunk_size = FIFO_CHUNK_SIZE * 3 / 4;
  std::vector<char> buf(chunk_size);
  size_t bytes_read;
  std::vector<char> temp_encoded;
  std::vector<char> left_over;

  // Open iconv descriptor once for all chunks (for stateful encodings like IBM-939)
  iconv_t cd = (iconv_t)(-1);
  string source_encoding;
  if (has_encoding)
  {
    source_encoding = strlen(zusf->encoding_opts.source_codepage) > 0 ? string(zusf->encoding_opts.source_codepage) : "UTF-8";
    cd = iconv_open(source_encoding.c_str(), encoding_to_use.c_str());
    if (cd == (iconv_t)(-1))
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Cannot open converter from %s to %s", encoding_to_use.c_str(), source_encoding.c_str());
      return RTNCD_FAILURE;
    }
  }

  while ((bytes_read = fread(&buf[0], 1, chunk_size, fin)) > 0)
  {
    int chunk_len = bytes_read;
    const char *chunk = &buf[0];

    if (has_encoding)
    {
      try
      {
        temp_encoded = zut_encode(chunk, chunk_len, cd, zusf->diag);
        chunk = &temp_encoded[0];
        chunk_len = temp_encoded.size();
      }
      catch (std::exception &e)
      {
        iconv_close(cd);
        zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from %s to %s", encoding_to_use.c_str(), source_encoding.c_str());
        return RTNCD_FAILURE;
      }
    }

    *content_len += chunk_len;
    temp_encoded = zbase64::encode(chunk, chunk_len, &left_over);
    fwrite(&temp_encoded[0], 1, temp_encoded.size(), fout);
    temp_encoded.clear();
  }

  // Flush the shift state for stateful encodings after all chunks are processed
  if (has_encoding && cd != (iconv_t)(-1))
  {
    try
    {
      // Flush the shift state
      std::vector<char> flush_buffer = zut_iconv_flush(cd, zusf->diag);
      if (flush_buffer.empty() && zusf->diag.e_msg_len > 0)
      {
        iconv_close(cd);
        return RTNCD_FAILURE;
      }

      // Write any shift sequence bytes that were generated
      if (!flush_buffer.empty())
      {
        *content_len += flush_buffer.size();
        temp_encoded = zbase64::encode(&flush_buffer[0], flush_buffer.size(), &left_over);
        fwrite(&temp_encoded[0], 1, temp_encoded.size(), fout);
      }
    }
    catch (std::exception &e)
    {
      iconv_close(cd);
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to flush encoding state");
      return RTNCD_FAILURE;
    }

    iconv_close(cd);
  }

  if (!left_over.empty())
  {
    temp_encoded = zbase64::encode(&left_over[0], left_over.size());
    fwrite(&temp_encoded[0], 1, temp_encoded.size(), fout);
  }

  fflush(fout);
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
int zusf_write_to_uss_file(ZUSF *zusf, const string &file, string &data)
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

  // Use encoding provided in arguments, otherwise fall back to file tag encoding
  string encoding_to_use;
  bool has_encoding = false;

  if (zusf->encoding_opts.data_type == eDataTypeText)
  {
    if (strlen(zusf->encoding_opts.codepage) > 0)
    {
      encoding_to_use = string(zusf->encoding_opts.codepage);
      has_encoding = true;
    }
    else
    {
      // Use tagged encoding if valid CCSID and not UTF-8 or binary
      int file_ccsid = zusf_get_file_ccsid(zusf, file);
      if (file_ccsid > 0 && file_ccsid != 1208 && file_ccsid != 65535)
      {
        encoding_to_use = zut_int_to_string(file_ccsid);
        has_encoding = true;
      }
    }
  }

  string temp = data;
  if (has_encoding)
  {
    const auto source_encoding = strlen(zusf->encoding_opts.source_codepage) > 0 ? string(zusf->encoding_opts.source_codepage) : "UTF-8";
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, source_encoding, encoding_to_use, zusf->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from %s to %s", source_encoding.c_str(), encoding_to_use.c_str());
      return RTNCD_FAILURE;
    }
  }

  AutocvtGuard autocvt(false);
  const char *mode = (zusf->encoding_opts.data_type == eDataTypeBinary) ? "wb" : "w";
  {
    FileGuard fp(file.c_str(), mode);
    if (!fp)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open '%s' for writing", file.c_str());
      return RTNCD_FAILURE;
    }

    if (!temp.empty())
    {
      size_t bytes_written = fwrite(temp.data(), 1, temp.size(), fp);
      if (bytes_written != temp.size())
      {
        zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to write to '%s' (possibly out of space)", file.c_str());
        return RTNCD_FAILURE;
      }
    }
  }

  if (has_encoding)
  {
    zusf_chtag_uss_file_or_dir(zusf, file, encoding_to_use, false);
  }

  struct stat new_stats;
  if (stat(file.c_str(), &new_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(
        zusf->diag.e_msg,
        "Could not stat file '%s' after writing",
        file.c_str());
    return RTNCD_FAILURE;
  }

  const string new_tag = zut_build_etag(new_stats.st_mtime, new_stats.st_size);
  strcpy(zusf->etag, new_tag.c_str());

  return RTNCD_SUCCESS; // success
}

/**
 * Writes data to a USS file.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file
 * @param pipe name of the input pipe
 * @param content_len pointer where the length of the file contents will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_write_to_uss_file_streamed(ZUSF *zusf, const string &file, const string &pipe, size_t *content_len)
{
  // TODO(zFernand0): Avoid overriding existing files
  if (content_len == nullptr)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "content_len must be a valid size_t pointer");
    return RTNCD_FAILURE;
  }

  struct stat file_stats;

  // Use encoding provided in arguments, otherwise fall back to file tag encoding
  string encoding_to_use;
  bool has_encoding = false;

  if (zusf->encoding_opts.data_type == eDataTypeText)
  {
    if (strlen(zusf->encoding_opts.codepage) > 0)
    {
      encoding_to_use = string(zusf->encoding_opts.codepage);
      has_encoding = true;
    }
    else
    {
      // Use tagged encoding if valid CCSID and not UTF-8 or binary
      int file_ccsid = zusf_get_file_ccsid(zusf, file);
      if (file_ccsid > 0 && file_ccsid != 1208 && file_ccsid != 65535)
      {
        encoding_to_use = zut_int_to_string(file_ccsid);
        has_encoding = true;
      }
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

  AutocvtGuard autocvt(false);
  FileGuard fout(file.c_str(), zusf->encoding_opts.data_type == eDataTypeBinary ? "wb" : "w");
  if (!fout)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open '%s'", file.c_str());
    return RTNCD_FAILURE;
  }

  int fifo_fd = open(pipe.c_str(), O_RDONLY);
  if (fifo_fd == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "open() failed on input pipe '%s', errno: %d", pipe.c_str(), errno);
    return RTNCD_FAILURE;
  }

  FileGuard fin(fifo_fd, "r");
  if (!fin)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open input pipe '%s'", pipe.c_str());
    close(fifo_fd);
    return RTNCD_FAILURE;
  }

  std::vector<char> buf(FIFO_CHUNK_SIZE);
  size_t bytes_read;
  std::vector<char> temp_encoded;
  std::vector<char> left_over;
  bool truncated = false;

  // Open iconv descriptor once for all chunks (for stateful encodings like IBM-939)
  iconv_t cd = (iconv_t)(-1);
  string source_encoding;
  if (has_encoding)
  {
    source_encoding = strlen(zusf->encoding_opts.source_codepage) > 0 ? string(zusf->encoding_opts.source_codepage) : "UTF-8";
    cd = iconv_open(encoding_to_use.c_str(), source_encoding.c_str());
    if (cd == (iconv_t)(-1))
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Cannot open converter from %s to %s", source_encoding.c_str(), encoding_to_use.c_str());
      return RTNCD_FAILURE;
    }
  }

  while ((bytes_read = fread(&buf[0], 1, FIFO_CHUNK_SIZE, fin)) > 0)
  {
    temp_encoded = zbase64::decode(&buf[0], bytes_read, &left_over);
    const char *chunk = &temp_encoded[0];
    int chunk_len = temp_encoded.size();
    *content_len += chunk_len;

    if (has_encoding)
    {
      try
      {
        temp_encoded = zut_encode(chunk, chunk_len, cd, zusf->diag);
        chunk = &temp_encoded[0];
        chunk_len = temp_encoded.size();
      }
      catch (std::exception &e)
      {
        iconv_close(cd);
        zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to convert input data from %s to %s", source_encoding.c_str(), encoding_to_use.c_str());
        return RTNCD_FAILURE;
      }
    }

    size_t bytes_written = fwrite(chunk, 1, chunk_len, fout);
    if (bytes_written != chunk_len)
    {
      truncated = true;
      break;
    }
    temp_encoded.clear();
  }

  // Flush the shift state for stateful encodings after all chunks are processed
  if (has_encoding && cd != (iconv_t)(-1))
  {
    try
    {
      // Flush the shift state
      std::vector<char> flush_buffer = zut_iconv_flush(cd, zusf->diag);
      if (flush_buffer.empty() && zusf->diag.e_msg_len > 0)
      {
        iconv_close(cd);
        return RTNCD_FAILURE;
      }

      // Write any shift sequence bytes that were generated
      if (!flush_buffer.empty())
      {
        size_t bytes_written = fwrite(&flush_buffer[0], 1, flush_buffer.size(), fout);
        if (bytes_written != flush_buffer.size())
        {
          truncated = true;
        }
      }
    }
    catch (std::exception &e)
    {
      iconv_close(cd);
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to flush encoding state");
      return RTNCD_FAILURE;
    }

    iconv_close(cd);
  }

  const int flush_rc = fflush(fout);
  if (truncated || flush_rc != 0)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to write to '%s' (possibly out of space)", file.c_str());
    return RTNCD_FAILURE;
  }

  if (has_encoding)
  {
    zusf_chtag_uss_file_or_dir(zusf, file, encoding_to_use, false);
  }

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
    DIR *dir = opendir(file.c_str());
    if (dir == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = zusf_join_path(file, string((const char *)entry->d_name));
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chmod_uss_file_or_dir(zusf, child_path, mode, S_ISDIR(file_stats.st_mode));
        if (0 != rc)
        {
          closedir(dir);
          return rc;
        }
      }
    }
    closedir(dir);
  }
  return 0;
}

int zusf_delete_uss_item(ZUSF *zusf, string file, bool recursive)
{
  struct stat file_stats;
  if (lstat(file.c_str(), &file_stats) == -1)
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
    DIR *dir = opendir(file.c_str());
    if (dir == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = zusf_join_path(file, string((const char *)entry->d_name));
        struct stat child_stats;
        if (lstat(child_path.c_str(), &child_stats) == -1)
        {
          closedir(dir);
          zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not stat child path '%s'", child_path.c_str());
          return RTNCD_FAILURE;
        }

        const auto rc = zusf_delete_uss_item(zusf, child_path, S_ISDIR(child_stats.st_mode));
        if (0 != rc)
        {
          closedir(dir);
          return rc;
        }
      }
    }
    closedir(dir);
  }

  const auto rc = is_dir ? rmdir(file.c_str()) : remove(file.c_str());
  if (0 != rc)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not delete '%s', rc: %d", file.c_str(), errno);
    return RTNCD_FAILURE;
  }

  return 0;
}

string zusf_get_owner_from_uid(uid_t uid)
{
  auto *meta = getpwuid(uid);
  return meta && meta->pw_name ? meta->pw_name : string();
}

string zusf_get_group_from_gid(gid_t gid)
{
  auto *meta = getgrgid(gid);
  return meta && meta->gr_name ? meta->gr_name : string();
}

short zusf_get_id_from_user_or_group(const string &user_or_group, bool is_user)
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

/**
 * Helper to convert user string to UID.
 * Accepts empty (→ -1), numeric UID, or name (getpwnam).
 * Returns true if resolved or empty, false if invalid/overflow.
 */
static bool resolve_uid_from_str(const std::string &s, uid_t &out)
{
  if (s.empty())
  {
    out = (uid_t)-1;
    return true;
  } // not user provided
  bool digits = s.find_first_not_of("0123456789") == std::string::npos;
  if (digits)
  {
    unsigned long v = strtoul(s.c_str(), nullptr, 10);
    if (v > std::numeric_limits<uid_t>::max())
      return false;
    out = static_cast<uid_t>(v);
    return true;
  }
  auto *pw = getpwnam(s.c_str());
  if (pw != nullptr)
  {
    out = pw->pw_uid;
    return true;
  }
  return false;
}

/**
 * Helper to convert group string to GID.
 * Accepts empty (→ -1), numeric GID, or name (getgrnam).
 * Returns true if resolved or empty, false if invalid/overflow.
 */
static bool resolve_gid_from_str(const std::string &s, gid_t &out)
{
  if (s.empty())
  {
    out = (gid_t)-1;
    return true;
  } // no group provided
  bool digits = s.find_first_not_of("0123456789") == std::string::npos;
  if (digits)
  {
    unsigned long v = strtoul(s.c_str(), nullptr, 10);
    if (v > std::numeric_limits<gid_t>::max())
      return false;
    out = static_cast<gid_t>(v);
    return true;
  }
  auto *gr = getgrnam(s.c_str());
  if (gr != nullptr)
  {
    out = gr->gr_gid;
    return true;
  }
  return false; // invalid group string
}

/**
 * Change ownership of a USS file or directory (recursive optional).
 *
 * Supports "user", "user:group", ":group", or numeric IDs.
 * Validates input, avoids silent -1, and returns RTNCD_FAILURE on error.
 */
int zusf_chown_uss_file_or_dir(ZUSF *zusf, const std::string &file, const std::string &owner, bool recursive)
{
  struct stat file_stats;
  // Verify target exists and capture current metadata
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  // Refuse to descend into a directory if caller didn’t request recursion
  if (S_ISDIR(file_stats.st_mode) && !recursive)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' is a folder and recursive is false", file.c_str());
    return RTNCD_FAILURE;
  }

  // Split owner into user[:group]
  std::string userPart = owner;
  std::string groupPart;
  const auto colon_pos = owner.find(':');
  if (colon_pos != std::string::npos)
  {
    userPart = owner.substr(0, colon_pos);
    groupPart = owner.substr(colon_pos + 1);
  }

  uid_t uid;
  gid_t gid;

  // Resolve user to UID (numeric or name); return error on invalid input
  if (!resolve_uid_from_str(userPart, uid))
  {
    errno = EINVAL;
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "chown error: invalid user '%s'", userPart.c_str());
    return RTNCD_FAILURE;
  }

  // Resolve group to GID (numeric or name); return error on invalid input
  if (!resolve_gid_from_str(groupPart, gid))
  {
    errno = EINVAL;
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "chown error: invalid group '%s'", groupPart.c_str());
    return RTNCD_FAILURE;
  }

  // If both were empty, refuse (otherwise chown(-1,-1) is a no-op)
  if (uid == (uid_t)-1 && gid == (gid_t)-1)
  {
    errno = EINVAL;
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "chown error: neither user nor group specified");
    return RTNCD_FAILURE;
  }

  // Preserve current group explicitly if only user was supplied
  if (gid == (gid_t)-1)
    gid = file_stats.st_gid;

  // Attempt chown
  const auto rc = chown(file.c_str(), uid, gid);
  if (rc != 0)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "chown failed for path '%s', errno %d", file.c_str(), errno);
    return RTNCD_FAILURE;
  }

  // Recurse into directories if requested
  if (recursive && S_ISDIR(file_stats.st_mode))
  {
    DIR *dir = opendir(file.c_str());
    if (!dir)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = zusf_join_path(file, string((const char *)entry->d_name));
        struct stat child_stats;
        if (stat(child_path.c_str(), &child_stats) == -1)
        {
          closedir(dir);
          zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not stat child path '%s'", child_path.c_str());
          return RTNCD_FAILURE;
        }

        const auto rc = zusf_chown_uss_file_or_dir(zusf, child_path, owner, S_ISDIR(child_stats.st_mode));
        if (rc != 0)
        {
          closedir(dir);
          return rc;
        }
      }
    }
    closedir(dir);
  }

  return 0;
}

int zusf_chtag_uss_file_or_dir(ZUSF *zusf, const string &file, const string &tag, bool recursive)
{
  struct stat file_stats;
  if (stat(file.c_str(), &file_stats) == -1)
  {
    zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Path '%s' does not exist", file.c_str());
    return RTNCD_FAILURE;
  }

  int ccsid;

  // First try to parse as a numeric CCSID
  char *endptr;
  const auto parsed_ccsid = strtol(tag.c_str(), &endptr, 10);
  // If the entire string was consumed and it's a valid range, it's a numeric CCSID
  if (*endptr == '\0' && parsed_ccsid != LONG_MAX && parsed_ccsid != LONG_MIN)
  {
    ccsid = parsed_ccsid;
  }
  else
  {
    // Try to get CCSID from display name
    ccsid = zusf_get_ccsid_from_display_name(tag);
    if (ccsid == -1)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Invalid tag '%s' - not a valid CCSID or display name", tag.c_str());
      return RTNCD_FAILURE;
    }
  }
  const auto is_dir = S_ISDIR(file_stats.st_mode);
  if (!is_dir)
  {
    attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.att_filetagchg = 1;
    attr.att_filetag.ft_ccsid = ccsid;
    attr.att_filetag.ft_txtflag = int(ccsid != 65535 && ccsid != 0);

    const auto rc = __chattr((char *)file.c_str(), &attr, sizeof(attr));
    if (0 != rc)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Failed to update attributes for path '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
  }
  else if (recursive)
  {
    DIR *dir = opendir(file.c_str());
    if (dir == nullptr)
    {
      zusf->diag.e_msg_len = sprintf(zusf->diag.e_msg, "Could not open directory '%s'", file.c_str());
      return RTNCD_FAILURE;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      {
        const string child_path = zusf_join_path(file, string((const char *)entry->d_name));
        struct stat file_stats;
        stat(child_path.c_str(), &file_stats);

        const auto rc = zusf_chtag_uss_file_or_dir(zusf, child_path, tag, S_ISDIR(file_stats.st_mode));
        if (0 != rc)
        {
          closedir(dir);
          return rc;
        }
      }
    }
    closedir(dir);
  }
  return 0;
}
