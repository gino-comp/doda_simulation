#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Error codes
typedef enum {
    DODA_SUCCESS = 0,
    DODA_ERROR_INVALID_INPUT = -1,
    DODA_ERROR_FILE_NOT_FOUND = -2,
    DODA_ERROR_COMPILATION_FAILED = -3,
    DODA_ERROR_MEMORY_ALLOCATION = -4,
    DODA_ERROR_UNKNOWN = -99
} doda_result_t;

// Opaque handle for compiler context
typedef struct doda_compiler_ctx* doda_compiler_handle_t;

// Bitstream structure for returning results
typedef struct {
    char** cluster_data;     // Array of strings, one per cluster
    size_t num_clusters;     // Number of clusters
    size_t* cluster_sizes;   // Size of each cluster's instruction array
} doda_bitstream_t;

// Runtime metadata structure
typedef struct {
    int input_size_bytes;
    int input_size_elements;
    int vector_size_check_passed;
} doda_runtime_metadata_t;

/**
 * Initialize a new compiler context
 * @return Handle to compiler context, or NULL on failure
 */
doda_compiler_handle_t doda_compiler_init(void);

/**
 * Clean up compiler context and free resources
 * @param handle Compiler context handle
 */
void doda_compiler_cleanup(doda_compiler_handle_t handle);

/**
 * Extract lambdas from C++ source and generate required files
 * @param handle Compiler context handle
 * @param source_path Path to C++ source file containing map_on_doda calls
 * @param output_dir Directory to place generated files (obj/ structure)
 * @return DODA_SUCCESS on success, error code on failure
 */
doda_result_t doda_extract_lambdas(
    doda_compiler_handle_t handle,
    const char* source_path,
    const char* output_dir
);

/**
 * Compile a DFG JSON file to bitstream
 * @param handle Compiler context handle
 * @param dfg_json_path Path to DFG JSON file
 * @param bitstream Output bitstream (caller must free with doda_free_bitstream)
 * @param metadata Output runtime metadata (optional, can be NULL)
 * @return DODA_SUCCESS on success, error code on failure
 */
doda_result_t doda_compile_dfg(
    doda_compiler_handle_t handle,
    const char* dfg_json_path,
    doda_bitstream_t* bitstream,
    doda_runtime_metadata_t* metadata
);

/**
 * Get the last error message from the compiler
 * @param handle Compiler context handle
 * @return Error message string (valid until next API call or cleanup)
 */
const char* doda_get_last_error(doda_compiler_handle_t handle);

/**
 * Free bitstream data allocated by the compiler
 * @param bitstream Bitstream to free
 */
void doda_free_bitstream(doda_bitstream_t* bitstream);

/**
 * Get library version information
 * @return Version string
 */
const char* doda_get_version(void);

#ifdef __cplusplus
}
#endif