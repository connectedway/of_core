/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/types.h"
#include "ofc/path.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/framework.h"

static OFC_INT test_startup(OFC_VOID) {
#if defined(INIT_ON_LOAD)
  volatile OFC_VOID *init = ofc_framework_init;
#else
    ofc_framework_init();
#endif
    return (0);
}

static OFC_VOID test_shutdown(OFC_VOID) {
#if !defined(INIT_ON_LOAD)
    ofc_framework_shutdown();
    ofc_framework_destroy();
#endif
}

TEST_GROUP(path);

TEST_SETUP(path) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(path) {
    test_shutdown();
}

TEST(path, test_path) {
    OFC_LPTSTR tfilename;
    OFC_LPSTR cfilename;
    OFC_LPTSTR tcursor;
    OFC_LPSTR ccursor;
    OFC_SIZET rem;
    OFC_SIZET trem;
    OFC_SIZET len;

    rem = 20;
    trem = rem;
    tfilename = ofc_malloc((trem + 1) * sizeof(OFC_TCHAR));
    tcursor = tfilename;

    len = ofc_path_make_urlW(&tcursor, &trem,
                             TSTR("COYOTE"),
                             TSTR("ROAD@RUNNER"),
                             TSTR("ACME"),
                             TSTR("BOOM"),
                             TSTR("DESERT"),
                             TSTR("dir1\\dir2\\dir3"),
                             TSTR("PAIN"));
    tfilename[rem] = TCHAR_EOS;

    ofc_printf("Truncated Path is %S\n", tfilename);
    ofc_free(tfilename);

    rem = len;
    trem = rem;
    tfilename = ofc_malloc((trem + 1) * sizeof(OFC_TCHAR));
    tcursor = tfilename;

    len = ofc_path_make_urlW(&tcursor, &trem,
                             TSTR("COYOTE"),
                             TSTR("ROAD@RUNNER"),
                             TSTR("ACME"),
                             TSTR("BOOM"),
                             TSTR("DESERT"),
                             TSTR("dir1\\dir2\\dir3"),
                             TSTR("PAIN"));
    tfilename[rem] = TCHAR_EOS;

    ofc_printf("Expanded Path is %S\n", tfilename);
    ofc_free(tfilename);

    rem = 20;
    trem = rem;
    cfilename = ofc_malloc((trem + 1) * sizeof(OFC_CHAR));
    ccursor = cfilename;

    len = ofc_path_make_urlA(&ccursor, &trem,
                             "COYOTE",
                             "ROAD@RUNNER",
                             "ACME",
                             "BOOM",
                             "DESERT",
                             "dir1\\dir2\\dir3",
                             "PAIN");
    cfilename[rem] = '\0';

    ofc_printf("Truncated Path is %s\n", cfilename);
    ofc_free(cfilename);

    rem = len;
    trem = rem;
    cfilename = ofc_malloc((trem + 1) * sizeof(OFC_CHAR));
    ccursor = cfilename;

    len = ofc_path_make_urlA(&ccursor, &trem,
                             "COYOTE",
                             "ROAD@RUNNER",
                             "ACME",
                             "BOOM",
                             "DESERT",
                             "dir1\\dir2\\dir3",
                             "PAIN");
    cfilename[rem] = '\0';

    ofc_printf("Expanded Path is %s\n", cfilename);

    ofc_free(cfilename);
}

TEST(path, test_path_insert)
{
  OFC_PATH *file_path;
  OFC_PATH *prefix;
  OFC_TCHAR *filename;

  file_path = ofc_path_createW(TSTR("/server/share/dir1/dir2/dir3"));

  prefix = ofc_path_createW(TSTR("/server2/share2/dir12"));
  ofc_path_promote_dirs(file_path, 2);

  for (OFC_UINT ix = 0 ; ix < ofc_path_num_dirs(prefix) ; ix++)
    {
      ofc_path_insert_dir (file_path, ix,
                           ofc_path_dir(prefix, ix));
    }

  filename = ofc_path_print_alloc(file_path);
  /*
   * filename should be /server2/share2/dir12/dir1/dir2/dir3
   */
  if (ofc_tstrcmp (filename,
                   TSTR("/server2/share2/dir12/dir1/dir2/dir3")) == 0)
    ofc_printf("insert path worked\n");
  else
    ofc_printf("insert path failed\n");
  ofc_free(filename);
  ofc_path_delete(file_path);
  ofc_path_delete(prefix);
}

TEST_GROUP_RUNNER(path) {
    RUN_TEST_CASE(path, test_path);
    RUN_TEST_CASE(path, test_path_insert);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(path);
}

int main(int argc, const char *argv[])
{
  return UnityMain(argc, argv, runAllTests);
}
#endif
