/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <string.h>
#if defined (_WIN32)
#include <windows.h>
#include <wincrypt.h>
#endif
#include "dcmtk_config.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/ofstd/ofstream.h"

#include "dcmtk_uid.h"

#if defined (_WIN32)
static bool
gen_random (unsigned char* buf, unsigned long buf_len)
{
    HCRYPTPROV h_cryptprov = NULL;
    LPTSTR prov_name = NULL;
    DWORD prov_name_len;
    BOOL rc;

    rc = CryptGetDefaultProvider (PROV_RSA_FULL, NULL, CRYPT_MACHINE_DEFAULT, 
	    NULL, &prov_name_len);
    if (!rc) {
	return false;
    }

    prov_name = (LPTSTR) LocalAlloc (LMEM_ZEROINIT, prov_name_len);
    if (!prov_name) {
	return false;
    }

    rc = CryptAcquireContext (&h_cryptprov, NULL, prov_name, 
	PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
    if (!rc) {
	LocalFree (prov_name);
	return false;
    }

    rc = CryptGenRandom (h_cryptprov, buf_len, buf);

    CryptReleaseContext (h_cryptprov, 0);
    LocalFree (prov_name);
    return ((bool) rc);
}

#else

static bool
gen_random (unsigned char* buf, unsigned long buf_len)
{
    return false;
}
#endif

/*  Unfortunately, the dcmtk uid generator suffers from non-randomness on 
    win32, because process id's are reused.  For example, here is the 
    output from win32 dicom_uid using the default generator:

    C:\gsharp\projects\plastimatch>dicom_uid & dicom_uid & dicom_uid
    1.2.276.0.7230010.3.1.4.1599324740.2152.1240860622.1
    1.2.276.0.7230010.3.1.4.1599324740.2680.1240860622.1
    1.2.276.0.7230010.3.1.4.1599324740.2152.1240860622.1

    This routine fixes this problem using the Win32 system cryptographic 
    entropy source (above).
*/
char*
dcmtk_uid (char *uid, const char *uid_root)
{
    int i;
    unsigned char random_buf[100];
    bool rc;
    int suppress_zeros = 1;

    dcmGenerateUniqueIdentifier (uid, uid_root);

    rc = gen_random (random_buf, 100);
    if (!rc) {
	return uid;
    }

    for (i = strlen (uid_root); i < 63; i++) {
	switch (uid[i]) {
	case '.':
	    suppress_zeros = 1;
	    break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    if (suppress_zeros) {
		uid[i] = '1' + ((((long) uid[i]) + random_buf[i] - '0') % 9);
	    } else {
		uid[i] = '0' + ((((long) uid[i]) + random_buf[i] - '0') % 10);
	    }
	    suppress_zeros = 0;
	    break;
	}
    }
    return uid;
}
