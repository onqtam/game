#pragma once

// generated with <repo>/scripts/python/gen_msg_macros.py

// clang-format off

#define HA_MSG_0(ns, r_t, n) namespace ns \
    { DYNAMIX_EXPORTED_MESSAGE_0(HAPI, r_t, n) }
#define HA_MSG_1(ns, r_t, n, a_0_t, a_0_n) namespace ns \
    { DYNAMIX_EXPORTED_MESSAGE_1(HAPI, r_t, n, a_0_t, a_0_n) }
#define HA_MSG_2(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n) namespace ns \
    { DYNAMIX_EXPORTED_MESSAGE_2(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n) }
#define HA_MSG_3(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n) namespace ns \
    { DYNAMIX_EXPORTED_MESSAGE_3(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n) }
#define HA_MSG_4(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n) namespace ns \
    { DYNAMIX_EXPORTED_MESSAGE_4(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n) }
#define HA_MSG_5(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n, a_4_t, a_4_n) namespace ns \
    { DYNAMIX_EXPORTED_MESSAGE_5(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n, a_4_t, a_4_n) }

#define HA_CONST_MSG_0(ns, r_t, n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, r_t, n) }
#define HA_CONST_MSG_1(ns, r_t, n, a_0_t, a_0_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MESSAGE_1(HAPI, r_t, n, a_0_t, a_0_n) }
#define HA_CONST_MSG_2(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MESSAGE_2(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n) }
#define HA_CONST_MSG_3(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MESSAGE_3(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n) }
#define HA_CONST_MSG_4(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MESSAGE_4(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n) }
#define HA_CONST_MSG_5(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n, a_4_t, a_4_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MESSAGE_5(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n, a_4_t, a_4_n) }

#define HA_MULTI_MSG_0(ns, r_t, n) namespace ns \
    { DYNAMIX_EXPORTED_MULTICAST_MESSAGE_0(HAPI, r_t, n) }
#define HA_MULTI_MSG_1(ns, r_t, n, a_0_t, a_0_n) namespace ns \
    { DYNAMIX_EXPORTED_MULTICAST_MESSAGE_1(HAPI, r_t, n, a_0_t, a_0_n) }
#define HA_MULTI_MSG_2(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n) namespace ns \
    { DYNAMIX_EXPORTED_MULTICAST_MESSAGE_2(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n) }
#define HA_MULTI_MSG_3(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n) namespace ns \
    { DYNAMIX_EXPORTED_MULTICAST_MESSAGE_3(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n) }
#define HA_MULTI_MSG_4(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n) namespace ns \
    { DYNAMIX_EXPORTED_MULTICAST_MESSAGE_4(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n) }
#define HA_MULTI_MSG_5(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n, a_4_t, a_4_n) namespace ns \
    { DYNAMIX_EXPORTED_MULTICAST_MESSAGE_5(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n, a_4_t, a_4_n) }

#define HA_CONST_MULTI_MSG_0(ns, r_t, n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_0(HAPI, r_t, n) }
#define HA_CONST_MULTI_MSG_1(ns, r_t, n, a_0_t, a_0_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(HAPI, r_t, n, a_0_t, a_0_n) }
#define HA_CONST_MULTI_MSG_2(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_2(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n) }
#define HA_CONST_MULTI_MSG_3(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_3(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n) }
#define HA_CONST_MULTI_MSG_4(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_4(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n) }
#define HA_CONST_MULTI_MSG_5(ns, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n, a_4_t, a_4_n) namespace ns \
    { DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_5(HAPI, r_t, n, a_0_t, a_0_n, a_1_t, a_1_n, a_2_t, a_2_n, a_3_t, a_3_n, a_4_t, a_4_n) }

#define HA_DEFINE_MSG(ns, n) namespace ns { DYNAMIX_DEFINE_MESSAGE(n); }

// clang-format on
