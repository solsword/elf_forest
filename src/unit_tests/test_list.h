#ifdef TEST_LIST_DEFINE

#include "suites/test_util.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_efd.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_color.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_list.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_queue.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_dictionary.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_map.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_map3.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_bitmap.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_blocks.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_tex.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_txgen.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_txg_plants.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_txg_minerals.h"
DEFINE_IMPORTED_BUILDER
#include "suites/test_worldgen.h"
DEFINE_IMPORTED_BUILDER
#endif // TEST_LIST_DEFINE

#ifdef TEST_LIST_SETUP
#include "suites/test_util.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_efd.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_color.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_blocks.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_tex.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_txgen.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_txg_plants.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_txg_minerals.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_dictionary.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
/*
#include "suites/test_worldgen.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
// Do data structure tests last 'cause they're slower:
#include "suites/test_list.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_queue.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_map.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_map3.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
#include "suites/test_bitmap.h"
ts = INVOKE_IMPORTED_BUILDER;
l_append_element(ALL_TEST_SUITES, ts);
// */

#endif // TEST_LIST_SETUP
