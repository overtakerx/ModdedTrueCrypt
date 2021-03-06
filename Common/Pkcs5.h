/*
 Legal Notice: The source code contained in this file has been derived from
 the source code of Encryption for the Masses 2.02a, which is Copyright (c)
 Paul Le Roux and which is covered by the 'License Agreement for Encryption
 for the Masses'. Modifications and additions to that source code contained
 in this file are Copyright (c) TrueCrypt Foundation and are covered by the
 TrueCrypt License 2.3 the full text of which is contained in the file
 License.txt included in TrueCrypt binary and source code distribution
 packages. */


void truncate (char *d1, char *d2, int len);
void hmac_sha1 (char *k, int lk, char *d, int ld, char *out, int t);
void derive_u_sha1 (char *pwd, int pwd_len, char *init, int salt_len, int iterations, char *u, int b);
void derive_key_sha1 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen);
void hmac_sha1Ex (sha1_ctx ictx, sha1_ctx octx, char *d, int ld, char *out);
void derive_u_sha1Ex (sha1_ctx ictx, sha1_ctx octx, char *salt, int salt_len, int iterations, char *u, int b);
void derive_key_sha1Ex (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen);
void hmac_ripemd160 (char *key, int keylen, char *input, int len, char *digest);
void derive_u_ripemd160 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *u, int b);
void derive_key_ripemd160 (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen);
void hmac_ripemd160Ex (char *input, int len, char *digest, RMD160_CTX ictx, RMD160_CTX octx);
void derive_u_ripemd160Ex (RMD160_CTX *ictx, RMD160_CTX *octx, char *init, int salt_len, int iterations, char *u, int b);
void derive_key_ripemd160Ex (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen);
void hmac_whirlpool (char *k, int lk, char *d, int ld, char *out, int t);
void derive_u_whirlpool (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *u, int b);
void derive_key_whirlpool (char *pwd, int pwd_len, char *salt, int salt_len, int iterations, char *dk, int dklen);
int get_pkcs5_iteration_count (int pkcs5_prf_id);
char *get_pkcs5_prf_name (int pkcs5_prf_id);
