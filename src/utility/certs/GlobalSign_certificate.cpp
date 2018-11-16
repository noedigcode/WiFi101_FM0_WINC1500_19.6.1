#include "GlobalSign_certificate.h"

#include "utility/certificate.h"


/*
   Validity Dates hardcoding taken from the output of 
     > openssl x509 -in GlobalSign.crt -inform der -text -noout
     Look for this section:
        Validity
            Not Before: Dec 12 00:00:00 2006 GMT
            Not After : Dec 12 00:00:00 2021 GMT
*/
static tstrSystemTime validFrom = {2006, 12, 15, 0, 0, 0};
static tstrSystemTime validUntil = {2021, 12, 15, 0, 0, 0};


/*
   Name Hash -- this one's a problem!  For the certificate represented
   here, I happen to know the sequence from previous runs of
   DumpWINC1500Certs.ino.  That won't be good enough for certificates
   that have not yet been on the WINC1500.  I have some ideas on how
   to generate it for future cases, but I'll come back to that...
   
   dumpRootCertV1: Name Hash:
   00000000: 0653 bd6d eab9 d58f 45d4 51ed 0325 0f65
   00000010: 5f86 593e
*/
#define NAME_HASH_LEN 20
static uint8 nameHash[NAME_HASH_LEN] = {
    0x06, 0x53, 0xbd, 0x6d, 0xea, 0xb9, 0xd5, 0x8f,
    0x45, 0xd4, 0x51, 0xed, 0x03, 0x25, 0x0f, 0x65,
    0x5f, 0x86, 0x59, 0x3e
};


/*
   PubKey hardcoding taken from the output of:
     > openssl x509 -in GlobalSign.crt -inform der -text -noout
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
    0xa6, 0xcf, 0x24, 0x0e, 0xbe, 0x2e, 0x6f, 0x28,
    0x99, 0x45, 0x42, 0xc4, 0xab, 0x3e, 0x21, 0x54,
    0x9b, 0x0b, 0xd3, 0x7f, 0x84, 0x70, 0xfa, 0x12,
    0xb3, 0xcb, 0xbf, 0x87, 0x5f, 0xc6, 0x7f, 0x86,
    0xd3, 0xb2, 0x30, 0x5c, 0xd6, 0xfd, 0xad, 0xf1,
    0x7b, 0xdc, 0xe5, 0xf8, 0x60, 0x96, 0x09, 0x92,
    0x10, 0xf5, 0xd0, 0x53, 0xde, 0xfb, 0x7b, 0x7e,
    0x73, 0x88, 0xac, 0x52, 0x88, 0x7b, 0x4a, 0xa6,
    0xca, 0x49, 0xa6, 0x5e, 0xa8, 0xa7, 0x8c, 0x5a,
    0x11, 0xbc, 0x7a, 0x82, 0xeb, 0xbe, 0x8c, 0xe9,
    0xb3, 0xac, 0x96, 0x25, 0x07, 0x97, 0x4a, 0x99,
    0x2a, 0x07, 0x2f, 0xb4, 0x1e, 0x77, 0xbf, 0x8a,
    0x0f, 0xb5, 0x02, 0x7c, 0x1b, 0x96, 0xb8, 0xc5,
    0xb9, 0x3a, 0x2c, 0xbc, 0xd6, 0x12, 0xb9, 0xeb,
    0x59, 0x7d, 0xe2, 0xd0, 0x06, 0x86, 0x5f, 0x5e,
    0x49, 0x6a, 0xb5, 0x39, 0x5e, 0x88, 0x34, 0xec,
    0xbc, 0x78, 0x0c, 0x08, 0x98, 0x84, 0x6c, 0xa8,
    0xcd, 0x4b, 0xb4, 0xa0, 0x7d, 0x0c, 0x79, 0x4d,
    0xf0, 0xb8, 0x2d, 0xcb, 0x21, 0xca, 0xd5, 0x6c,
    0x5b, 0x7d, 0xe1, 0xa0, 0x29, 0x84, 0xa1, 0xf9,
    0xd3, 0x94, 0x49, 0xcb, 0x24, 0x62, 0x91, 0x20,
    0xbc, 0xdd, 0x0b, 0xd5, 0xd9, 0xcc, 0xf9, 0xea,
    0x27, 0x0a, 0x2b, 0x73, 0x91, 0xc6, 0x9d, 0x1b,
    0xac, 0xc8, 0xcb, 0xe8, 0xe0, 0xa0, 0xf4, 0x2f,
    0x90, 0x8b, 0x4d, 0xfb, 0xb0, 0x36, 0x1b, 0xf6,
    0x19, 0x7a, 0x85, 0xe0, 0x6d, 0xf2, 0x61, 0x13,
    0x88, 0x5c, 0x9f, 0xe0, 0x93, 0x0a, 0x51, 0x97,
    0x8a, 0x5a, 0xce, 0xaf, 0xab, 0xd5, 0xf7, 0xaa,
    0x09, 0xaa, 0x60, 0xbd, 0xdc, 0xd9, 0x5f, 0xdf,
    0x72, 0xa9, 0x60, 0x13, 0x5e, 0x00, 0x01, 0xc9,
    0x4a, 0xfa, 0x3f, 0xa4, 0xea, 0x07, 0x03, 0x21,
    0x02, 0x8e, 0x82, 0xca, 0x03, 0xc2, 0x9b, 0x8f
};


const Certificate *GlobalSign_getCertificate()
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


