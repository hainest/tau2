#ifndef _DDM_H_
#define _DDM_H_

/* dalvik extened JDWP command */
#define DDM_TRUNK 0xc701

/* DDM Trunk Types */
#define DDM_HELO 0x48454c4f
#define DDM_THEN 0x5448454e
#define DDM_THCR 0x54484352
#define DDM_THDE 0x54484445

/* Generic DDM Trunk */
typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t length;

    char     data[];
} ddm_trunk_t;

/* DDM Chunk THEN */
typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t length;

    char enable;
} ddm_then_t;

/* DDM Chunk THCR */
typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t length;

    uint32_t lid;        /* VM-local thread ID (usually a small int */
    uint32_t tname_len;  /* thread name len (in 16-bit chars        */
    char     tname[];    /* thread name (UTF-16                     */
} ddm_thcr_t;

/* DDM Chunk THDE */
typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t length;

    uint32_t lid;        /* VM-local thread ID (usually a small int */
} ddm_thde_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//int ddm_trunk_type(char *name);
int ddm_helo(jdwp_ctx_t* jdwp);
int ddm_then(jdwp_ctx_t* jdwp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
