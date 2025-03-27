// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Primary model header
//
// This header should be included by all source files instantiating the design.
// The class here is then constructed to instantiate the design.
// See the Verilator manual for examples.

#ifndef VERILATED_VDODA_H_
#define VERILATED_VDODA_H_  // guard

#include "verilated.h"

class VDODA_PSN017;
class VDODA_PS2ryv;
class VDODA_PSUXCz;


// This class is the main interface to the Verilated model
class alignas(VL_CACHE_LINE_BYTES) VDODA VL_NOT_FINAL : public VerilatedModel {
  private:
    // Symbol table holding complete model state (owned by this class)
    VDODA_PSN017* const vlSymsp;

  public:

    // PORTS
    // The application code writes and reads these signals to
    // propagate new values into/out from the Verilated model.
    VL_IN8(&clock,0,0);
    VL_IN8(&reset,0,0);
    VL_IN8(&io_init,0,0);
    VL_OUT8(&io_ready_to_run,0,0);
    VL_OUT8(&io_status,2,0);
    VL_OUT8(&io_out_read_done,0,0);
    VL_OUT8(&io_v_inst_prog_in_0_ready,0,0);
    VL_IN8(&io_v_inst_prog_in_0_valid,0,0);
    VL_OUT8(&io_v_inst_prog_in_1_ready,0,0);
    VL_IN8(&io_v_inst_prog_in_1_valid,0,0);
    VL_OUT8(&io_v_inst_prog_in_2_ready,0,0);
    VL_IN8(&io_v_inst_prog_in_2_valid,0,0);
    VL_OUT8(&io_v_inst_prog_in_3_ready,0,0);
    VL_IN8(&io_v_inst_prog_in_3_valid,0,0);
    VL_OUT8(&io_v_inst_prog_in_4_ready,0,0);
    VL_IN8(&io_v_inst_prog_in_4_valid,0,0);
    VL_OUT8(&io_v_inst_prog_in_5_ready,0,0);
    VL_IN8(&io_v_inst_prog_in_5_valid,0,0);
    VL_OUT8(&io_v_inst_prog_in_6_ready,0,0);
    VL_IN8(&io_v_inst_prog_in_6_valid,0,0);
    VL_OUT8(&io_v_inst_prog_in_7_ready,0,0);
    VL_IN8(&io_v_inst_prog_in_7_valid,0,0);
    VL_IN8(&io_v_in_prog_read_done_0,0,0);
    VL_IN8(&io_v_in_prog_read_done_1,0,0);
    VL_IN8(&io_v_in_prog_read_done_2,0,0);
    VL_IN8(&io_v_in_prog_read_done_3,0,0);
    VL_IN8(&io_v_in_prog_read_done_4,0,0);
    VL_IN8(&io_v_in_prog_read_done_5,0,0);
    VL_IN8(&io_v_in_prog_read_done_6,0,0);
    VL_IN8(&io_v_in_prog_read_done_7,0,0);
    VL_OUT8(&io_v_t_axi_read_in_0_ready,0,0);
    VL_IN8(&io_v_t_axi_read_in_0_valid,0,0);
    VL_OUT8(&io_v_t_axi_read_in_1_ready,0,0);
    VL_IN8(&io_v_t_axi_read_in_1_valid,0,0);
    VL_OUT8(&io_v_t_axi_read_in_2_ready,0,0);
    VL_IN8(&io_v_t_axi_read_in_2_valid,0,0);
    VL_OUT8(&io_v_t_axi_read_in_3_ready,0,0);
    VL_IN8(&io_v_t_axi_read_in_3_valid,0,0);
    VL_OUT8(&io_v_t_axi_read_in_4_ready,0,0);
    VL_IN8(&io_v_t_axi_read_in_4_valid,0,0);
    VL_OUT8(&io_v_t_axi_read_in_5_ready,0,0);
    VL_IN8(&io_v_t_axi_read_in_5_valid,0,0);
    VL_OUT8(&io_v_t_axi_read_in_6_ready,0,0);
    VL_IN8(&io_v_t_axi_read_in_6_valid,0,0);
    VL_OUT8(&io_v_t_axi_read_in_7_ready,0,0);
    VL_IN8(&io_v_t_axi_read_in_7_valid,0,0);
    VL_IN8(&io_v_t_axi_write_out_0_ready,0,0);
    VL_OUT8(&io_v_t_axi_write_out_0_valid,0,0);
    VL_IN8(&io_v_t_axi_write_out_1_ready,0,0);
    VL_OUT8(&io_v_t_axi_write_out_1_valid,0,0);
    VL_IN8(&io_v_t_axi_write_out_2_ready,0,0);
    VL_OUT8(&io_v_t_axi_write_out_2_valid,0,0);
    VL_IN8(&io_v_t_axi_write_out_3_ready,0,0);
    VL_OUT8(&io_v_t_axi_write_out_3_valid,0,0);
    VL_IN8(&io_v_t_axi_write_out_4_ready,0,0);
    VL_OUT8(&io_v_t_axi_write_out_4_valid,0,0);
    VL_IN8(&io_v_t_axi_write_out_5_ready,0,0);
    VL_OUT8(&io_v_t_axi_write_out_5_valid,0,0);
    VL_IN8(&io_v_t_axi_write_out_6_ready,0,0);
    VL_OUT8(&io_v_t_axi_write_out_6_valid,0,0);
    VL_IN8(&io_v_t_axi_write_out_7_ready,0,0);
    VL_OUT8(&io_v_t_axi_write_out_7_valid,0,0);
    VL_IN8(&io_v_in_spm_read_done_0,0,0);
    VL_IN8(&io_v_in_spm_read_done_1,0,0);
    VL_IN8(&io_v_in_spm_read_done_2,0,0);
    VL_IN8(&io_v_in_spm_read_done_3,0,0);
    VL_IN8(&io_v_in_spm_read_done_4,0,0);
    VL_IN8(&io_v_in_spm_read_done_5,0,0);
    VL_IN8(&io_v_in_spm_read_done_6,0,0);
    VL_IN8(&io_v_in_spm_read_done_7,0,0);
    VL_INW(&io_v_inst_prog_in_0_bits,255,0,8);
    VL_INW(&io_v_inst_prog_in_1_bits,255,0,8);
    VL_INW(&io_v_inst_prog_in_2_bits,255,0,8);
    VL_INW(&io_v_inst_prog_in_3_bits,255,0,8);
    VL_INW(&io_v_inst_prog_in_4_bits,255,0,8);
    VL_INW(&io_v_inst_prog_in_5_bits,255,0,8);
    VL_INW(&io_v_inst_prog_in_6_bits,255,0,8);
    VL_INW(&io_v_inst_prog_in_7_bits,255,0,8);
    VL_IN(&io_v_t_axi_read_in_0_bits,31,0);
    VL_IN(&io_v_t_axi_read_in_1_bits,31,0);
    VL_IN(&io_v_t_axi_read_in_2_bits,31,0);
    VL_IN(&io_v_t_axi_read_in_3_bits,31,0);
    VL_IN(&io_v_t_axi_read_in_4_bits,31,0);
    VL_IN(&io_v_t_axi_read_in_5_bits,31,0);
    VL_IN(&io_v_t_axi_read_in_6_bits,31,0);
    VL_IN(&io_v_t_axi_read_in_7_bits,31,0);
    VL_OUT(&io_v_t_axi_write_out_0_bits,31,0);
    VL_OUT(&io_v_t_axi_write_out_1_bits,31,0);
    VL_OUT(&io_v_t_axi_write_out_2_bits,31,0);
    VL_OUT(&io_v_t_axi_write_out_3_bits,31,0);
    VL_OUT(&io_v_t_axi_write_out_4_bits,31,0);
    VL_OUT(&io_v_t_axi_write_out_5_bits,31,0);
    VL_OUT(&io_v_t_axi_write_out_6_bits,31,0);
    VL_OUT(&io_v_t_axi_write_out_7_bits,31,0);

    // CELLS
    // Public to allow access to /* verilator public */ items.
    // Otherwise the application code can consider these internals.
    VDODA_PSUXCz* const PSPQKp;
    VDODA_PSUXCz* const PSiB0E;
    VDODA_PSUXCz* const PSIMUH;
    VDODA_PSUXCz* const PSCLXT;
    VDODA_PSUXCz* const PSHxZz;
    VDODA_PSUXCz* const PS7VN2;
    VDODA_PSUXCz* const PS2JAb;
    VDODA_PSUXCz* const PSdU7z;

    // Root instance pointer to allow access to model internals,
    // including inlined /* verilator public_flat_* */ items.
    VDODA_PS2ryv* const rootp;

    // CONSTRUCTORS
    /// Construct the model; called by application code
    /// If contextp is null, then the model will use the default global context
    /// If name is "", then makes a wrapper with a
    /// single model invisible with respect to DPI scope names.
    explicit VDODA(VerilatedContext* contextp, const char* name = "TOP");
    explicit VDODA(const char* name = "TOP");
    /// Destroy the model; called (often implicitly) by application code
    virtual ~VDODA();
  private:
    VL_UNCOPYABLE(VDODA);  ///< Copying not allowed

  public:
    // API METHODS
    /// Evaluate the model.  Application must call when inputs change.
    void eval() { eval_step(); }
    /// Evaluate when calling multiple units/models per time step.
    void eval_step();
    /// Evaluate at end of a timestep for tracing, when using eval_step().
    /// Application must call after all eval() and before time changes.
    void eval_end_step() {}
    /// Simulation complete, run final blocks.  Application must call on completion.
    void final();
    /// Are there scheduled events to handle?
    bool eventsPending();
    /// Returns time at next time slot. Aborts if !eventsPending()
    uint64_t nextTimeSlot();
    /// Trace signals in the model; called by application code
    void trace(VerilatedVcdC* tfp, int levels, int options = 0);
    /// Retrieve name of this model instance (as passed to constructor).
    const char* name() const;

    // Abstract methods from VerilatedModel
    const char* hierName() const override final;
    const char* modelName() const override final;
    unsigned threads() const override final;
    /// Prepare for cloning the model at the process level (e.g. fork in Linux)
    /// Release necessary resources. Called before cloning.
    void prepareClone() const;
    /// Re-init after cloning the model at the process level (e.g. fork in Linux)
    /// Re-allocate necessary resources. Called after cloning.
    void atClone() const;
};

#endif  // guard
