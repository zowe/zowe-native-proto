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
#include <unordered_map>
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

using namespace std;

// CCSID -> display name conversion table (based on output from `iconv -l`)
static std::tr1::unordered_map<int, string> ccsid_map = {
    {37, "IBM-037"},
    {256, "00256"},
    {259, "00259"},
    {273, "IBM-273"},
    {274, "IBM-274"},
    {275, "IBM-275"},
    {277, "IBM-277"},
    {278, "IBM-278"},
    {280, "IBM-280"},
    {281, "IBM-281"},
    {282, "IBM-282"},
    {284, "IBM-284"},
    {285, "IBM-285"},
    {286, "00286"},
    {290, "IBM-290"},
    {293, "00293"},
    {297, "IBM-297"},
    {300, "IBM-300"},
    {301, "IBM-301"},
    {367, "00367"},
    {420, "IBM-420"},
    {421, "00421"},
    {423, "00423"},
    {424, "IBM-424"},
    {425, "IBM-425"},
    {437, "IBM-437"},
    {500, "IBM-500"},
    {720, "00720"},
    {737, "00737"},
    {775, "00775"},
    {803, "00803"},
    {806, "00806"},
    {808, "IBM-808"},
    {813, "ISO8859-7"},
    {819, "ISO8859-1"},
    {833, "IBM-833"},
    {834, "IBM-834"},
    {835, "IBM-835"},
    {836, "IBM-836"},
    {837, "IBM-837"},
    {838, "IBM-838"},
    {848, "IBM-848"},
    {849, "00849"},
    {850, "IBM-850"},
    {851, "00851"},
    {852, "IBM-852"},
    {853, "00853"},
    {855, "IBM-855"},
    {856, "IBM-856"},
    {857, "00857"},
    {858, "IBM-858"},
    {859, "IBM-859"},
    {860, "00860"},
    {861, "IBM-861"},
    {862, "IBM-862"},
    {863, "00863"},
    {864, "IBM-864"},
    {865, "00865"},
    {866, "IBM-866"},
    {867, "IBM-867"},
    {868, "00868"},
    {869, "IBM-869"},
    {870, "IBM-870"},
    {871, "IBM-871"},
    {872, "IBM-872"},
    {874, "TIS-620"},
    {875, "IBM-875"},
    {876, "00876"},
    {878, "00878"},
    {880, "IBM-880"},
    {891, "00891"},
    {895, "00895"},
    {896, "00896"},
    {897, "00897"},
    {899, "00899"},
    {901, "IBM-901"},
    {902, "IBM-902"},
    {903, "00903"},
    {904, "IBM-904"},
    {905, "00905"},
    {912, "ISO8859-2"},
    {913, "00913"},
    {914, "ISO8859-4"},
    {915, "ISO8859-5"},
    {916, "ISO8859-8"},
    {918, "00918"},
    {920, "ISO8859-9"},
    {921, "ISO8859-13"},
    {922, "IBM-922"},
    {923, "ISO8859-15"},
    {924, "IBM-924"},
    {926, "00926"},
    {927, "IBM-927"},
    {928, "IBM-928"},
    {930, "IBM-930"},
    {931, "00931"},
    {932, "IBM-eucJC"},
    {933, "IBM-933"},
    {934, "00934"},
    {935, "IBM-935"},
    {936, "IBM-936"},
    {937, "IBM-937"},
    {938, "IBM-938"},
    {939, "IBM-939"},
    {941, "00941"},
    {942, "IBM-942"},
    {943, "IBM-943"},
    {944, "00944"},
    {946, "IBM-946"},
    {947, "IBM-947"},
    {948, "IBM-948"},
    {949, "IBM-949"},
    {950, "BIG5"},
    {951, "IBM-951"},
    {952, "00952"},
    {953, "00953"},
    {954, "00954"},
    {955, "00955"},
    {956, "IBM-956"},
    {957, "IBM-957"},
    {958, "IBM-958"},
    {959, "IBM-959"},
    {960, "00960"},
    {961, "00961"},
    {963, "00963"},
    {964, "IBM-eucTW"},
    {965, "00965"},
    {966, "00966"},
    {970, "IBM-eucKR"},
    {971, "00971"},
    {1002, "01002"},
    {1004, "01004"},
    {1006, "01006"},
    {1008, "01008"},
    {1009, "01009"},
    {1010, "01010"},
    {1011, "01011"},
    {1012, "01012"},
    {1013, "01013"},
    {1014, "01014"},
    {1015, "01015"},
    {1016, "01016"},
    {1017, "01017"},
    {1018, "01018"},
    {1019, "01019"},
    {1020, "01020"},
    {1021, "01021"},
    {1023, "01023"},
    {1025, "IBM-1025"},
    {1026, "IBM-1026"},
    {1027, "IBM-1027"},
    {1040, "01040"},
    {1041, "01041"},
    {1042, "01042"},
    {1043, "01043"},
    {1046, "IBM-1046"},
    {1047, "IBM-1047"},
    {1051, "01051"},
    {1088, "IBM-1088"},
    {1089, "ISO8859-6"},
    {1097, "01097"},
    {1098, "01098"},
    {1100, "01100"},
    {1101, "01101"},
    {1102, "01102"},
    {1103, "01103"},
    {1104, "01104"},
    {1105, "01105"},
    {1106, "01106"},
    {1107, "01107"},
    {1112, "IBM-1112"},
    {1114, "01114"},
    {1115, "IBM-1115"},
    {1122, "IBM-1122"},
    {1123, "IBM-1123"},
    {1124, "IBM-1124"},
    {1125, "IBM-1125"},
    {1126, "IBM-1126"},
    {1129, "01129"},
    {1130, "01130"},
    {1131, "01131"},
    {1132, "01132"},
    {1133, "01133"},
    {1137, "01137"},
    {1140, "IBM-1140"},
    {1141, "IBM-1141"},
    {1142, "IBM-1142"},
    {1143, "IBM-1143"},
    {1144, "IBM-1144"},
    {1145, "IBM-1145"},
    {1146, "IBM-1146"},
    {1147, "IBM-1147"},
    {1148, "IBM-1148"},
    {1149, "IBM-1149"},
    {1153, "IBM-1153"},
    {1154, "IBM-1154"},
    {1155, "IBM-1155"},
    {1156, "IBM-1156"},
    {1157, "IBM-1157"},
    {1158, "IBM-1158"},
    {1159, "IBM-1159"},
    {1160, "IBM-1160"},
    {1161, "IBM-1161"},
    {1162, "01162"},
    {1163, "01163"},
    {1164, "01164"},
    {1165, "IBM-1165"},
    {1166, "01166"},
    {1167, "01167"},
    {1168, "01168"},
    {1200, "01200"},
    {1202, "01202"},
    {1208, "UTF-8"},
    {1210, "01210"},
    {1232, "01232"},
    {1250, "IBM-1250"},
    {1251, "IBM-1251"},
    {1252, "IBM-1252"},
    {1253, "IBM-1253"},
    {1254, "IBM-1254"},
    {1255, "IBM-1255"},
    {1256, "IBM-1256"},
    {1257, "01257"},
    {1258, "01258"},
    {1275, "01275"},
    {1276, "01276"},
    {1277, "01277"},
    {1280, "01280"},
    {1281, "01281"},
    {1282, "01282"},
    {1283, "01283"},
    {1284, "01284"},
    {1285, "01285"},
    {1287, "01287"},
    {1288, "01288"},
    {1350, "01350"},
    {1351, "01351"},
    {1362, "IBM-1362"},
    {1363, "IBM-1363"},
    {1364, "IBM-1364"},
    {1370, "IBM-1370"},
    {1371, "IBM-1371"},
    {1374, "01374"},
    {1375, "01375"},
    {1380, "IBM-1380"},
    {1381, "IBM-1381"},
    {1382, "01382"},
    {1383, "IBM-eucCN"},
    {1385, "01385"},
    {1386, "IBM-1386"},
    {1388, "IBM-1388"},
    {1390, "IBM-1390"},
    {1391, "01391"},
    {1392, "01392"},
    {1399, "IBM-1399"},
    {4133, "04133"},
    {4369, "04369"},
    {4370, "04370"},
    {4371, "04371"},
    {4373, "04373"},
    {4374, "04374"},
    {4376, "04376"},
    {4378, "04378"},
    {4380, "04380"},
    {4381, "04381"},
    {4386, "04386"},
    {4393, "04393"},
    {4396, "IBM-4396"},
    {4397, "04397"},
    {4516, "04516"},
    {4517, "04517"},
    {4519, "04519"},
    {4520, "04520"},
    {4533, "04533"},
    {4596, "04596"},
    {4899, "04899"},
    {4904, "04904"},
    {4909, "IBM-4909"},
    {4929, "04929"},
    {4930, "IBM-4930"},
    {4931, "04931"},
    {4932, "04932"},
    {4933, "IBM-4933"},
    {4934, "04934"},
    {4944, "04944"},
    {4945, "04945"},
    {4946, "IBM-4946"},
    {4947, "04947"},
    {4948, "04948"},
    {4949, "04949"},
    {4951, "04951"},
    {4952, "04952"},
    {4953, "04953"},
    {4954, "04954"},
    {4955, "04955"},
    {4956, "04956"},
    {4957, "04957"},
    {4958, "04958"},
    {4959, "04959"},
    {4960, "04960"},
    {4961, "04961"},
    {4962, "04962"},
    {4963, "04963"},
    {4964, "04964"},
    {4965, "04965"},
    {4966, "04966"},
    {4967, "04967"},
    {4970, "04970"},
    {4971, "IBM-4971"},
    {4976, "04976"},
    {4992, "04992"},
    {4993, "04993"},
    {5012, "05012"},
    {5014, "05014"},
    {5023, "05023"},
    {5026, "IBM-5026"},
    {5028, "05028"},
    {5029, "05029"},
    {5031, "IBM-5031"},
    {5033, "05033"},
    {5035, "IBM-5035"},
    {5038, "05038"},
    {5039, "05039"},
    {5043, "05043"},
    {5045, "05045"},
    {5046, "05046"},
    {5047, "05047"},
    {5048, "05048"},
    {5049, "05049"},
    {5050, "05050"},
    {5052, "ISO-2022-JP"},
    {5053, "IBM-5053"},
    {5054, "IBM-5054"},
    {5055, "IBM-5055"},
    {5056, "05056"},
    {5067, "05067"},
    {5100, "05100"},
    {5104, "05104"},
    {5123, "IBM-5123"},
    {5137, "05137"},
    {5142, "05142"},
    {5143, "05143"},
    {5210, "05210"},
    {5211, "05211"},
    {5346, "IBM-5346"},
    {5347, "IBM-5347"},
    {5348, "IBM-5348"},
    {5349, "IBM-5349"},
    {5350, "IBM-5350"},
    {5351, "IBM-5351"},
    {5352, "IBM-5352"},
    {5353, "05353"},
    {5354, "05354"},
    {5470, "05470"},
    {5471, "05471"},
    {5472, "05472"},
    {5473, "05473"},
    {5476, "05476"},
    {5477, "05477"},
    {5478, "05478"},
    {5479, "05479"},
    {5486, "05486"},
    {5487, "05487"},
    {5488, "IBM-5488"},
    {5495, "05495"},
    {8229, "08229"},
    {8448, "08448"},
    {8482, "IBM-8482"},
    {8492, "08492"},
    {8493, "08493"},
    {8612, "08612"},
    {8629, "08629"},
    {8692, "08692"},
    {9025, "09025"},
    {9026, "09026"},
    {9027, "IBM-9027"},
    {9028, "09028"},
    {9030, "09030"},
    {9042, "09042"},
    {9044, "IBM-9044"},
    {9047, "09047"},
    {9048, "09048"},
    {9049, "09049"},
    {9056, "09056"},
    {9060, "09060"},
    {9061, "IBM-9061"},
    {9064, "09064"},
    {9066, "09066"},
    {9088, "09088"},
    {9089, "09089"},
    {9122, "09122"},
    {9124, "09124"},
    {9125, "09125"},
    {9127, "09127"},
    {9131, "09131"},
    {9139, "09139"},
    {9142, "09142"},
    {9144, "09144"},
    {9145, "09145"},
    {9146, "09146"},
    {9163, "09163"},
    {9238, "IBM-9238"},
    {9306, "09306"},
    {9444, "09444"},
    {9447, "09447"},
    {9448, "09448"},
    {9449, "09449"},
    {9572, "09572"},
    {9574, "09574"},
    {9575, "09575"},
    {9577, "09577"},
    {9580, "09580"},
    {12544, "12544"},
    {12588, "12588"},
    {12712, "IBM-12712"},
    {12725, "12725"},
    {12788, "12788"},
    {13121, "IBM-13121"},
    {13124, "IBM-13124"},
    {13125, "13125"},
    {13140, "13140"},
    {13143, "13143"},
    {13145, "13145"},
    {13152, "13152"},
    {13156, "13156"},
    {13157, "13157"},
    {13162, "13162"},
    {13184, "13184"},
    {13185, "13185"},
    {13218, "13218"},
    {13219, "13219"},
    {13221, "13221"},
    {13223, "13223"},
    {13235, "13235"},
    {13238, "13238"},
    {13240, "13240"},
    {13241, "13241"},
    {13242, "13242"},
    {13488, "UCS-2"},
    {13671, "13671"},
    {13676, "13676"},
    {16421, "16421"},
    {16684, "IBM-16684"},
    {16804, "IBM-16804"},
    {16821, "16821"},
    {16884, "16884"},
    {17221, "17221"},
    {17240, "17240"},
    {17248, "IBM-17248"},
    {17314, "17314"},
    {17331, "17331"},
    {17337, "17337"},
    {17354, "17354"},
    {17584, "17584"},
    {20517, "20517"},
    {20780, "20780"},
    {20917, "20917"},
    {20980, "20980"},
    {21314, "21314"},
    {21317, "21317"},
    {21344, "21344"},
    {21427, "21427"},
    {21433, "21433"},
    {21450, "21450"},
    {21680, "21680"},
    {24613, "24613"},
    {24876, "24876"},
    {24877, "24877"},
    {25013, "25013"},
    {25076, "25076"},
    {25426, "25426"},
    {25427, "25427"},
    {25428, "25428"},
    {25429, "25429"},
    {25431, "25431"},
    {25432, "25432"},
    {25433, "25433"},
    {25436, "25436"},
    {25437, "25437"},
    {25438, "25438"},
    {25439, "25439"},
    {25440, "25440"},
    {25441, "25441"},
    {25442, "25442"},
    {25444, "25444"},
    {25445, "25445"},
    {25450, "25450"},
    {25467, "25467"},
    {25473, "25473"},
    {25479, "25479"},
    {25480, "25480"},
    {25502, "25502"},
    {25503, "25503"},
    {25504, "25504"},
    {25508, "25508"},
    {25510, "25510"},
    {25512, "25512"},
    {25514, "25514"},
    {25518, "25518"},
    {25520, "25520"},
    {25522, "25522"},
    {25524, "25524"},
    {25525, "25525"},
    {25527, "25527"},
    {25546, "25546"},
    {25580, "25580"},
    {25616, "25616"},
    {25617, "25617"},
    {25618, "25618"},
    {25619, "25619"},
    {25664, "25664"},
    {25690, "25690"},
    {25691, "25691"},
    {28709, "IBM-28709"},
    {29109, "29109"},
    {29172, "29172"},
    {29522, "29522"},
    {29523, "29523"},
    {29524, "29524"},
    {29525, "29525"},
    {29527, "29527"},
    {29528, "29528"},
    {29529, "29529"},
    {29532, "29532"},
    {29533, "29533"},
    {29534, "29534"},
    {29535, "29535"},
    {29536, "29536"},
    {29537, "29537"},
    {29540, "29540"},
    {29541, "29541"},
    {29546, "29546"},
    {29614, "29614"},
    {29616, "29616"},
    {29618, "29618"},
    {29620, "29620"},
    {29621, "29621"},
    {29623, "29623"},
    {29712, "29712"},
    {29713, "29713"},
    {29714, "29714"},
    {29715, "29715"},
    {29760, "29760"},
    {32805, "32805"},
    {33058, "33058"},
    {33205, "33205"},
    {33268, "33268"},
    {33618, "33618"},
    {33619, "33619"},
    {33620, "33620"},
    {33621, "33621"},
    {33623, "33623"},
    {33624, "33624"},
    {33632, "33632"},
    {33636, "33636"},
    {33637, "33637"},
    {33665, "33665"},
    {33698, "33698"},
    {33699, "33699"},
    {33700, "33700"},
    {33717, "33717"},
    {33722, "EUCJP"},
    {37301, "37301"},
    {37719, "37719"},
    {37728, "37728"},
    {37732, "37732"},
    {37761, "37761"},
    {37813, "37813"},
    {41397, "41397"},
    {41460, "41460"},
    {41824, "41824"},
    {41828, "41828"},
    {45493, "45493"},
    {45556, "45556"},
    {45920, "45920"},
    {49589, "49589"},
    {49652, "49652"},
    {53668, "IBM-53668"},
    {53685, "53685"},
    {53748, "53748"},
    {54189, "54189"},
    {54191, "IBM-54191"},
    {54289, "54289"},
    {61696, "61696"},
    {61697, "61697"},
    {61698, "61698"},
    {61699, "61699"},
    {61700, "61700"},
    {61710, "61710"},
    {61711, "61711"},
    {61712, "61712"},
    {61953, "61953"},
    {61956, "61956"},
    {62337, "62337"},
    {62381, "62381"},
    {62383, "IBM-62383"},
    {65535, "binary"}};

/**
 * Gets the display name for a CCSID.
 * @param ccsid the CCSID value
 * @return display name string for the CCSID, or the CCSID number as a string if not found
 */
string zusf_get_ccsid_display_name(int ccsid)
{
  // Special case for invalid/unset CCSID
  if (ccsid <= 0)
  {
    return "untagged";
  }

  // O(log n) lookup in map
  auto it = ccsid_map.find(ccsid);
  if (it != ccsid_map.end())
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
 * Lists the USS file path.
 *
 * @param zusf pointer to a ZUSF object
 * @param file name of the USS file or directory
 * @param response reference to a string where the read data will be stored
 *
 * @return RTNCD_SUCCESS on success, RTNCD_FAILURE on failure
 */
int zusf_list_uss_file_path(ZUSF *zusf, string file, string &response, ListOptions options)
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
    response.clear();
    const auto file_name = file.substr(file.find_last_of("/") + 1);
    vector<string> fields;
    fields.push_back(file_name);
    if (options.long_format)
    {
      const string mode = zusf_build_mode_string(file_stats.st_mode);
      const auto ccsid = zusf_get_ccsid_display_name(zusf_get_file_ccsid(zusf, file));
      fields.push_back(mode);
      fields.push_back(ccsid);
    }
    response = zut_format_as_csv(fields) + "\n";
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

  struct dirent *entry;
  response.clear();
  while ((entry = readdir(dir)) != nullptr)
  {
    if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
    {
      string child_path = file[file.length() - 1] == '/' ? file + string(entry->d_name) : file + string("/") + string(entry->d_name);
      struct stat child_stats;
      stat(child_path.c_str(), &child_stats);

      vector<string> fields;
      string name = entry->d_name;
      if (name.at(0) == '.' && !options.all_files)
      {
        continue;
      }

      fields.push_back(entry->d_name);
      if (options.long_format)
      {
        const string mode = zusf_build_mode_string(child_stats.st_mode);
        const auto ccsid = zusf_get_ccsid_display_name(child_stats.st_tag.ft_ccsid);
        fields.push_back(mode);
        fields.push_back(ccsid);
      }
      response += zut_format_as_csv(fields) + "\n";
    }
  }
  closedir(dir);

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

  if (size > 0 && has_encoding)
  {
    std::string temp = response;
    try
    {
      const auto bytes_with_encoding = zut_encode(temp, encoding_to_use, "UTF-8", zusf->diag);
      temp = bytes_with_encoding;
    }
    catch (std::exception &e)
    {
      // TODO: error handling
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
