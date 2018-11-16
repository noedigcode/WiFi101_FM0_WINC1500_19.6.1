#include "NMARoot_certificate.h"

#include "utility/certificate.h"


/*
   Validity Dates hardcoding taken from the output of 
     > openssl x509 -in NMA_Root.cer -inform der -text -noout
     Look for this section:
        Validity
            Not Before: May 30 10:48:38 2000 GMT
            Not After : May 30 10:48:38 2020 GMT
*/
static tstrSystemTime validFrom = {2000, 5, 30, 10, 48, 38};
static tstrSystemTime validUntil = {2020, 5, 30, 10, 48, 38};


/*
   Name Hash -- this one's a problem!  For the certificate represented
   here, I happen to know the sequence from previous runs of
   DumpWINC1500Certs.ino.  That won't be good enough for certificates
   that have not yet been on the WINC1500.  I have some ideas on how
   to generate it for future cases, but I'll come back to that...
   
   dumpRootCertV1: Name Hash:
   00000000: 42ca cf1c 2884 dafb c77e ac5d 0975 3d63
   00000010: 1efa ad7d
*/
#define NAME_HASH_LEN 20
static uint8 nameHash[NAME_HASH_LEN] = {
    0x42, 0xca, 0xcf, 0x1c, 0x28, 0x84, 0xda, 0xfb,
    0xc7, 0x7e, 0xac, 0x5d, 0x09, 0x75, 0x3d, 0x63,
    0x1e, 0xfa, 0xad, 0x7d
};


/*
   PubKey hardcoding taken from the output of:
     > openssl x509 -in NMA_Root.cer -inform der -text -noout
     Look for this section:
            Public Key Algorithm: rsaEncryption
                Public-Key: (2048 bit)
                Modulus:
                    00:b7:f7:1a:33:e6:f2:00:04:2d:39:e0:4e:5b:ed:
                    1f:bc:6c:0f:cd:b5:fa:23:b6:ce:de:9b:11:33:97:
                    a4:29:4c:7d:93:9f:bd:4a:bc:93:ed:03:1a:e3:8f:

     IMPORTANT -- omit the leading 00 reported by openssl (I'm not sure why
     it's putting it there, but all the following data indeed adds up to
     2048 bits or 256 bytes).  The extra byte has been confirmed as a
     spurious report by checking on the contents on the WINC1500 using
     DumpWINC1500Certs for other certificates compared to the openssl report.
*/
#define EXPONENT_LEN 3
static uint8 exponentBuf[EXPONENT_LEN] = {0x01, 0x00, 0x01}; // Exponent of 65537 == 0x010001
#define MODULUS_LEN 256
static uint8 pubkey[MODULUS_LEN] = {
    0xb7, 0xf7, 0x1a, 0x33, 0xe6, 0xf2, 0x00, 0x04,
    0x2d, 0x39, 0xe0, 0x4e, 0x5b, 0xed, 0x1f, 0xbc,
    0x6c, 0x0f, 0xcd, 0xb5, 0xfa, 0x23, 0xb6, 0xce,
    0xde, 0x9b, 0x11, 0x33, 0x97, 0xa4, 0x29, 0x4c,
    0x7d, 0x93, 0x9f, 0xbd, 0x4a, 0xbc, 0x93, 0xed,
    0x03, 0x1a, 0xe3, 0x8f, 0xcf, 0xe5, 0x6d, 0x50,
    0x5a, 0xd6, 0x97, 0x29, 0x94, 0x5a, 0x80, 0xb0,
    0x49, 0x7a, 0xdb, 0x2e, 0x95, 0xfd, 0xb8, 0xca,
    0xbf, 0x37, 0x38, 0x2d, 0x1e, 0x3e, 0x91, 0x41,
    0xad, 0x70, 0x56, 0xc7, 0xf0, 0x4f, 0x3f, 0xe8,
    0x32, 0x9e, 0x74, 0xca, 0xc8, 0x90, 0x54, 0xe9,
    0xc6, 0x5f, 0x0f, 0x78, 0x9d, 0x9a, 0x40, 0x3c,
    0x0e, 0xac, 0x61, 0xaa, 0x5e, 0x14, 0x8f, 0x9e,
    0x87, 0xa1, 0x6a, 0x50, 0xdc, 0xd7, 0x9a, 0x4e,
    0xaf, 0x05, 0xb3, 0xa6, 0x71, 0x94, 0x9c, 0x71,
    0xb3, 0x50, 0x60, 0x0a, 0xc7, 0x13, 0x9d, 0x38,
    0x07, 0x86, 0x02, 0xa8, 0xe9, 0xa8, 0x69, 0x26,
    0x18, 0x90, 0xab, 0x4c, 0xb0, 0x4f, 0x23, 0xab,
    0x3a, 0x4f, 0x84, 0xd8, 0xdf, 0xce, 0x9f, 0xe1,
    0x69, 0x6f, 0xbb, 0xd7, 0x42, 0xd7, 0x6b, 0x44,
    0xe4, 0xc7, 0xad, 0xee, 0x6d, 0x41, 0x5f, 0x72,
    0x5a, 0x71, 0x08, 0x37, 0xb3, 0x79, 0x65, 0xa4,
    0x59, 0xa0, 0x94, 0x37, 0xf7, 0x00, 0x2f, 0x0d,
    0xc2, 0x92, 0x72, 0xda, 0xd0, 0x38, 0x72, 0xdb,
    0x14, 0xa8, 0x45, 0xc4, 0x5d, 0x2a, 0x7d, 0xb7,
    0xb4, 0xd6, 0xc4, 0xee, 0xac, 0xcd, 0x13, 0x44,
    0xb7, 0xc9, 0x2b, 0xdd, 0x43, 0x00, 0x25, 0xfa,
    0x61, 0xb9, 0x69, 0x6a, 0x58, 0x23, 0x11, 0xb7,
    0xa7, 0x33, 0x8f, 0x56, 0x75, 0x59, 0xf5, 0xcd,
    0x29, 0xd7, 0x46, 0xb7, 0x0a, 0x2b, 0x65, 0xb6,
    0xd3, 0x42, 0x6f, 0x15, 0xb2, 0xb8, 0x7b, 0xfb,
    0xef, 0xe9, 0x5d, 0x53, 0xd5, 0x34, 0x5a, 0x27
};


const Certificate *NMARoot_getCertificate()
{
  static Certificate certificate;
  static int initialized = 0;
  
  if (!initialized) {
    /* Load Certificate details into the struct */
    m2m_memcpy(certificate.nameHash, nameHash, CRYPTO_SHA1_DIGEST_SIZE);
    certificate.validFrom = validFrom;
    certificate.validUntil = validUntil;
    certificate.pubKeyType = X509_CERT_PUBKEY_RSA;
    certificate.pubkey.u16NSize = MODULUS_LEN;
    certificate.pubkey.u16ESize = EXPONENT_LEN;
    certificate.pubkey.pu8N = pubkey;
    certificate.pubkey.pu8E = exponentBuf;

    initialized = 1;
  }

  return &certificate;
}


