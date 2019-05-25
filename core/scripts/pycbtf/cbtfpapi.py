#!/usr/bin/env python


def are_not_all_zeros(results):
    """Checks for test results that are all zeroes so we can skip them."""
    if results[0] == 0 and results[1] == 0 and results[2] == 0:
        return False
    else:
        return True


def papi_calculations(data, ret):
    """Calculates comp intensity and instruct per cycle"""
    if 'PAPI_TOT_INS' in data and 'PAPI_TOT_CYC' in data:
        results1 = [instruct / float(cycles) if cycles != 0 else 0
                    for instruct, cycles in
                    zip(data['PAPI_TOT_INS'], data['PAPI_TOT_CYC'])]
        if are_not_all_zeros(results1):
            ret.append(['instructions_per_cycle', results1])
        results2 = [cycles / float(instruct) if instruct != 0 else 0
                    for instruct, cycles in
                    zip(data['PAPI_TOT_INS'], data['PAPI_TOT_CYC'])]
        if are_not_all_zeros(results2):
            ret.append(['computational_intensity', results2])


def processor_frequency(data, ret):
    """Calculates processor core base frequency"""
    if 'PAPI_REF_CYC' in data and 'PAPI_REF_NS' in data:
        results = [ref_cyc / float(ref_ns) if ref_ns != 0 else 0
                   for ref_cyc, ref_ns in
                   zip(data['PAPI_REF_CYC'], data['PAPI_REF_NS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['processor_core_base_freq', results])


def flop_rate(data, ret):
    """Calculates the float point operations per second"""
    if 'PAPI_DP_OPS' in data and 'PAPI_FDV_INS' in data \
        and 'PAPI_REF_NS' in data:
        results = [(dp_ops + fdv_ins) / float(ref_ns) if ref_ns != 0 else 0
                   for dp_ops, fdv_ins, ref_ns in
                   zip(data['PAPI_DP_OPS'], data['PAPI_FDV_INS'],
                       data['PAPI_REF_NS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['FLOP_rate', results])


def proc_turbo_ratio(data, ret):
    """Calculates the processor turbo rate"""
    if 'PAPI_TOT_CYC' in data and 'PAPI_REF_CYC' in data:
        results = [tot_cyc / float(ref_cyc) if ref_cyc != 0 else 0
                   for tot_cyc, ref_cyc in
                   zip(data['PAPI_TOT_CYC'], data['PAPI_REF_CYC'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['processor_turbo_rate', results])


def proc_turbo_frequency(data, ret):
    """Calculates the processor turbo frequency"""
    if 'PAPI_TOTAL_CYC' in data and 'PAPI_REF_NS' in data:
        results = [tot_cyc / float(ref_ns) if ref_ns != 0 else 0
                   for tot_cyc, ref_ns in
                   zip(data['PAPI_TOTAL_CYC'], data['PAPI_REF_NS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['processor_turbo_freq', results])


def ratio_of_load_instructions(data, ret):
    """Calculates the ratio of load instructions"""
    if 'PAPI_LD_INS' in data and 'PAPI_TOT_INS' in data:
        results = [ld_ins / float(tot_ins) if tot_ins != 0 else 0
                   for ld_ins, tot_ins in
                   zip(data['PAPI_LD_INS'], data['PAPI_TOT_INS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['ratio_of_load_instruct', results])


def time_per_load_instruction(data, ret):
    """Calculates time per load instruction"""
    if 'PAPI_REF_NS' in data and 'PAPI_LD_INS' in data:
        results = [ref_ns / float(ld_ins) if ld_ins != 0 else 0
                   for ref_ns, ld_ins in
                   zip(data['PAPI_REF_NS'], data['PAPI_LD_INS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['time_per_load_instruct', results])


def vectorization_rate(data, ret):
    """Calculates the vectorization rate"""
    if 'PAPI_VEC_DP' in data and 'PAPI_DP_OPS' in data \
        and 'PAPI_FDV_OPS' in data:
        results = [vec_dp / float(dp_ops + fdv_ops) if dp_ops != 0 else 0
                   for vec_dp, dp_ops, fdv_ops in
                   zip(data['PAPI_VEC_DP'], data['PAPI_DP_OPS'],
                       data['PAPI_FDV_OPS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['vectorization_rate', results])


def ratio_of_floating_point_instruct(data, ret):
    """Calculates the ratio of floating point instructions"""
    if 'PAPI_DP_OPS' in data and 'PAPI_TOT_INS' in data:
        results = [dp_ops / float(tot_ins) if tot_ins != 0 else 0
                   for dp_ops, tot_ins in
                   zip(data['PAPI_DP_OPS'], data['PAPI_TOT_INS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['ratio_of_fp_instruct', results])


def floating_point_intensity(data, ret):
    """Calculates the floating point intensity"""
    if 'PAPI_DP_OPS' in data and 'PAPI_LD_INS' in data:
        results = [dp_ops / float(ld_ins) if ld_ins != 0 else 0
                   for dp_ops, ld_ins in
                   zip(data['PAPI_DP_OPS'], data['PAPI_LD_INS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['float_point_intensity', results])


def memory_hits_per_load(data, ret):
    """Calculates the memory hits per load"""
    if 'PAPI_L3_TCM' in data and 'PAP_LD_INS' in data:
        results = [l3_tcm / float(ld_ins) if ld_ins != 0 else 0
                   for l3_tcm, ld_ins in
                   zip(data['PAPI_L3_TCM'], data['PAPI_LD_INS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['mem_hits_per_load', results])


def memory_hits_per_instruct(data, ret):
    """Calculates the memory hits per instruction"""
    if 'PAPI_L3_TCM' in data and 'PAPI_TOT_INS' in data:
        results = [l3_tcm / float(tot_ins) if tot_ins != 0 else 0
                   for l3_tcm, tot_ins in
                   zip(data['PAPI_L3_TCM'], data['PAPI_TOT_INS'])]
        if are_not_all_zeros(results) != 0:
            ret.append(['mem_hits_per_instruct', results])


def l3_computations(data, ret):
    """
    Calculates L3 cache hits, L3 cache hits per load, and 
    L3 cache hits per instruct
    """
    if 'PAPI_L2_TCM' in data and 'PAPI_L3_TCM' in data:
        results1 = [l2_tcm - l3_tcm for l2_tcm, l3_tcm in
                    zip(data['PAPI_L2_TCM'], data['PAPI_L3_TCM'])]
        if are_not_all_zeros(results1) != 0:
            ret.append(['L3_cache_hits', results1])

        if 'PAPI_LD_INS' in data:
            results2 = [(l2_tcm - l3_tcm) / float(ld_ins) if ld_ins != 0 else 0
                        for l2_tcm, l3_tcm, ld_ins in
                        zip(data['PAPI_L2_TCM'], data['PAPI_L3_TCM'],
                            data['PAPI_LD_INS'])]
            if are_not_all_zeros(results2) != 0:
                ret.append(['L3_cache_hits_per_load', results2])

        if 'PAPI_TOT_INS' in data:
            results3 = [
                (l2_tcm - l3_tcm) / float(tot_ins) if tot_ins != 0 else 0
                for l2_tcm, l3_tcm, tot_ins in
                zip(data['PAPI_L2_TCM'], data['PAPI_L3_TCM'],
                    data['PAPI_TOT_INS'])]
            if are_not_all_zeros(results3) != 0:
                ret.append(['L3_cache_hits_per_instruct', results3])


def l2_computations(data, ret):
    """ 
    Calculates L2 cache hits, L2 cache hits per load and 
    L2 cache hits per instruction
    """
    if 'PAPI_L1_TCM' in data and 'PAPI_L2_TCM' in data:
        results1 = [l1_tcm - l2_tcm for l1_tcm, l2_tcm in
                    zip(data['PAPI_L1_TCM'], data['PAPI_L2_TCM'])]
        if are_not_all_zeros(results1) != 0:
            ret.append(['L2_cache_hits', results1])

        if 'PAPI_LD_INS' in data:
            results2 = [(l1_tcm - l2_tcm) / float(ld_ins) if ld_ins != 0 else 0
                        for l1_tcm, l2_tcm, ld_ins in
                        zip(data['PAPI_L1_TCM'], data['PAPI_L2_TCM'],
                            data['PAPI_LD_INS'])]
            if are_not_all_zeros(results2) != 0:
                ret.append(['L2_cache_hits_per_load', results2])

        if 'PAPI_TOT_INS' in data:
            results3 = [
                (l1_tcm - l2_tcm) / float(tot_ins) if tot_ins != 0 else 0
                for l1_tcm, l2_tcm, tot_ins in
                zip(data['PAPI_L1_TCM'], data['PAPI_L2_TCM'],
                    data['PAPI_TOT_INS'])]
            if are_not_all_zeros(results3) != 0:
                ret.append(['L2_cache_hits_per_instruct', results3])


def process_calculations(data):
    """ Master function that calls all calculations"""
    ret = []
    papi_calculations(data, ret)
    processor_frequency(data, ret)
    flop_rate(data, ret)
    proc_turbo_ratio(data, ret)
    proc_turbo_frequency(data, ret)
    ratio_of_load_instructions(data, ret)
    time_per_load_instruction(data, ret)
    vectorization_rate(data, ret)
    ratio_of_floating_point_instruct(data, ret)
    floating_point_intensity(data, ret)
    memory_hits_per_load(data, ret)
    memory_hits_per_instruct(data, ret)
    l3_computations(data, ret)
    l2_computations(data, ret)
    return ret
