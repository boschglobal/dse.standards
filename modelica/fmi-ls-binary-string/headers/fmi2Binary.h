// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MODELICA_FMI_LS_BINARY_STRING_HEADERS_FMI2BINARY_H_
#define MODELICA_FMI_LS_BINARY_STRING_HEADERS_FMI2BINARY_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "fmi2Functions.h"

#define fmi2GetBinary               fmi2FullName(fmi2GetBinary)
#define fmi2SetBinary               fmi2FullName(fmi2SetBinary)

FMI2_Export fmi2GetBinaryTYPE  fmi2GetBinary;
FMI2_Export fmi2SetBinaryTYPE  fmi2SetBinary;

typedef const fmi2Byte* fmi2Binary;

typedef fmi2Status fmi2GetBinaryTYPE(fmi2Component c,
    const fmi2ValueReference vr[], size_t nvr,
    size_t valueSize[], fmi2Binary value[], size_t nValues);
typedef fmi2Status fmi2SetBinaryTYPE(fmi2Component c,
    const fmi2ValueReference vr[], size_t nvr,
    const size_t valueSize[], const fmi2Binary value[], size_t nValues);


#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif // MODELICA_FMI_LS_BINARY_STRING_HEADERS_FMI2BINARY_H_
