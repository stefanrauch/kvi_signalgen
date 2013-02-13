/*
  Register definitions for slave core: BuTis clock generator

  * File           : wb_BuTiSclock.c
  * Author         : auto-generated by wbgen2 from gen_BuTiSclock.wb
  * Created        : 09/20/12 15:35:38
  * Standard       : ANSI C

    THIS FILE WAS GENERATED BY wbgen2 FROM SOURCE FILE gen_BuTiSclock.wb
    DO NOT HAND-EDIT UNLESS IT'S ABSOLUTELY NECESSARY!

*/

#ifndef __WBGEN2_REGDEFS_GEN_BUTISCLOCK_WB
#define __WBGEN2_REGDEFS_GEN_BUTISCLOCK_WB

#include <inttypes.h>

#if defined( __GNUC__)
#define PACKED __attribute__ ((packed))
#else
#error "Unsupported compiler?"
#endif

#ifndef __WBGEN2_MACROS_DEFINED__
#define __WBGEN2_MACROS_DEFINED__
#define WBGEN2_GEN_MASK(offset, size) (((1<<(size))-1) << (offset))
#define WBGEN2_GEN_WRITE(value, offset, size) (((value) & ((1<<(size))-1)) << (offset))
#define WBGEN2_GEN_READ(reg, offset, size) (((reg) >> (offset)) & ((1<<(size))-1))
#define WBGEN2_SIGN_EXTEND(value, bits) (((value) & (1<<bits) ? ~((1<<(bits))-1): 0 ) | (value))
#endif


/* definitions for register: TimeStamp data Low word */

/* definitions for field: Low Word in reg: TimeStamp data Low word */
#define WBBUTIS_TIMESTAMP_LW_MASK             WBGEN2_GEN_MASK(0, 32)
#define WBBUTIS_TIMESTAMP_LW_SHIFT            0
#define WBBUTIS_TIMESTAMP_LW_W(value)         WBGEN2_GEN_WRITE(value, 0, 32)
#define WBBUTIS_TIMESTAMP_LW_R(reg)           WBGEN2_GEN_READ(reg, 0, 32)

/* definitions for register: TimeStamp data High word */

/* definitions for field: High Word in reg: TimeStamp data High word */
#define WBBUTIS_TIMESTAMP_HW_MASK             WBGEN2_GEN_MASK(0, 32)
#define WBBUTIS_TIMESTAMP_HW_SHIFT            0
#define WBBUTIS_TIMESTAMP_HW_W(value)         WBGEN2_GEN_WRITE(value, 0, 32)
#define WBBUTIS_TIMESTAMP_HW_R(reg)           WBGEN2_GEN_READ(reg, 0, 32)

/* definitions for register: BuTis clock generator control */

/* definitions for field: Set on next PPS in reg: BuTis clock generator control */
#define WBBUTIS_CONTROL_SET_MASK              WBGEN2_GEN_MASK(0, 1)
#define WBBUTIS_CONTROL_SET_SHIFT             0
#define WBBUTIS_CONTROL_SET_W(value)          WBGEN2_GEN_WRITE(value, 0, 1)
#define WBBUTIS_CONTROL_SET_R(reg)            WBGEN2_GEN_READ(reg, 0, 1)

/* definitions for field: Re-synchronize in reg: BuTis clock generator control */
#define WBBUTIS_CONTROL_SYNC_MASK             WBGEN2_GEN_MASK(1, 1)
#define WBBUTIS_CONTROL_SYNC_SHIFT            1
#define WBBUTIS_CONTROL_SYNC_W(value)         WBGEN2_GEN_WRITE(value, 1, 1)
#define WBBUTIS_CONTROL_SYNC_R(reg)           WBGEN2_GEN_READ(reg, 1, 1)

/* definitions for field: reset phase-PLL in reg: BuTis clock generator control */
#define WBBUTIS_CONTROL_RESET_MASK            WBGEN2_GEN_MASK(2, 1)
#define WBBUTIS_CONTROL_RESET_SHIFT           2
#define WBBUTIS_CONTROL_RESET_W(value)        WBGEN2_GEN_WRITE(value, 2, 1)
#define WBBUTIS_CONTROL_RESET_R(reg)          WBGEN2_GEN_READ(reg, 2, 1)

/* definitions for field: unused in reg: BuTis clock generator control */
#define WBBUTIS_CONTROL_UNUSED_MASK           WBGEN2_GEN_MASK(3, 5)
#define WBBUTIS_CONTROL_UNUSED_SHIFT          3
#define WBBUTIS_CONTROL_UNUSED_W(value)       WBGEN2_GEN_WRITE(value, 3, 5)
#define WBBUTIS_CONTROL_UNUSED_R(reg)         WBGEN2_GEN_READ(reg, 3, 5)

/* definitions for field: PLLphase in reg: BuTis clock generator control */
#define WBBUTIS_CONTROL_PHASE_MASK            WBGEN2_GEN_MASK(8, 8)
#define WBBUTIS_CONTROL_PHASE_SHIFT           8
#define WBBUTIS_CONTROL_PHASE_W(value)        WBGEN2_GEN_WRITE(value, 8, 8)
#define WBBUTIS_CONTROL_PHASE_R(reg)          WBGEN2_GEN_READ(reg, 8, 8)

/* definitions for register: BuTis clock generator Status */

/* definitions for field: timestamp set busy in reg: BuTis clock generator Status */
#define WBBUTIS_STATUS_SET_MASK               WBGEN2_GEN_MASK(0, 1)
#define WBBUTIS_STATUS_SET_SHIFT              0
#define WBBUTIS_STATUS_SET_W(value)           WBGEN2_GEN_WRITE(value, 0, 1)
#define WBBUTIS_STATUS_SET_R(reg)             WBGEN2_GEN_READ(reg, 0, 1)

/* definitions for field: Phase of the PPS in reg: BuTis clock generator Status */
#define WBBUTIS_STATUS_PPSPHASE_MASK          WBGEN2_GEN_MASK(1, 1)
#define WBBUTIS_STATUS_PPSPHASE_SHIFT         1
#define WBBUTIS_STATUS_PPSPHASE_W(value)      WBGEN2_GEN_WRITE(value, 1, 1)
#define WBBUTIS_STATUS_PPSPHASE_R(reg)        WBGEN2_GEN_READ(reg, 1, 1)

PACKED struct WBBUTIS_WB {
  /* [0x0]: REG TimeStamp data Low word */
  uint32_t TIMESTAMP;
  /* [0x4]: REG TimeStamp data High word */
  uint32_t TIMESTAMP;
  /* [0x8]: REG BuTis clock generator control */
  uint32_t CONTROL;
  /* [0xc]: REG BuTis clock generator Status */
  uint32_t STATUS;
};

#endif
