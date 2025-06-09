#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <stdexcept>

class DodaCompiler {
public:
    // Compiler exceptions
    class CompilationError : public std::runtime_error {
    public:
        explicit CompilationError(const std::string& msg) : std::runtime_error(msg) {}
    };

    class RuntimeError : public std::runtime_error {
    public:
        explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
    };

    // Bitstream class - contains generated bitstream for a lambda
    class Bitstream {
    private:
        std::vector<std::vector<std::string>> data_;  // [cluster][pe] -> binary instruction
        std::string error_message_;
        bool valid_;

    public:
        Bitstream() : valid_(false) {}
        
        // Constructor from mapper bitstream data
        Bitstream(std::vector<std::vector<std::string>> bitstream_data) 
            : data_(std::move(bitstream_data)), valid_(true) {}
        
        // Constructor for error case
        Bitstream(const std::string& error_msg) 
            : error_message_(error_msg), valid_(false) {}

        // Accessors
        bool is_valid() const { return valid_; }
        const std::string& error() const { return error_message_; }
        
        int num_clusters() const { return valid_ ? data_.size() : 0; }
        int pes_per_cluster() const { return (valid_ && !data_.empty()) ? data_[0].size() : 0; }
        
        // Get instruction for specific cluster and PE
        const std::string& get_instruction(int cluster, int pe) const {
            static const std::string empty_string;
            
            if (!valid_ || cluster < 0 || pe < 0 || 
                cluster >= static_cast<int>(data_.size()) ||
                pe >= static_cast<int>(data_[cluster].size())) {
                return empty_string;
            }
            
            return data_[cluster][pe];
        }
        
        // Export to file (optional convenience method)
        bool export_to_file(const std::string& filename) const;
        
        // Debug print
        void print_summary() const;
    };

    // Compilation result containing compiled lambdas
    class CompileResult {
    public:
        // Execute a lambda function
        uint32_t execute_lambda(int lambda_index, uint32_t input) const;
        
        // Get DFG JSON for a lambda
        const std::string& get_dfg_json(int lambda_index) const;
        
        // Get number of compiled lambdas
        int get_lambda_count() const;
        
        // Get work directory path
        const std::string& get_work_dir() const;
        
        // Generate bitstream for a lambda
        std::unique_ptr<Bitstream> generate_bitstream(int lambda_index) const;
        
        // Internal constructor (used by DodaCompiler)
        CompileResult(const std::string& work_dir, int lambda_count);
        
    private:
        std::string work_dir_;
        int lambda_count_;
        mutable std::vector<std::string> dfg_cache_;  // Lazy-loaded DFG JSONs
        void* library_handle_;  // dlopen handle
        
        void load_library();
        void load_dfg(int lambda_index) const;
        
        friend class DodaCompiler;
    };

public:
    // Constructor - creates compiler with specified work directory
    explicit DodaCompiler(const std::string& work_dir = "/tmp/doda_compiler");
    
    // Destructor
    ~DodaCompiler();
    
    // Delete copy constructor and assignment (move-only)
    DodaCompiler(const DodaCompiler&) = delete;
    DodaCompiler& operator=(const DodaCompiler&) = delete;
    
    // Move constructor and assignment
    DodaCompiler(DodaCompiler&& other) noexcept;
    DodaCompiler& operator=(DodaCompiler&& other) noexcept;
    
    // Compile source code containing map_on_doda calls
    // Returns CompileResult on success, throws CompilationError on failure
    std::unique_ptr<CompileResult> compile_source(const std::string& source_code,
                                                 const std::string& source_filename);
    
    // Get compiler work directory
    const std::string& get_work_dir() const { return work_dir_; }

private:
    std::string work_dir_;
    std::string container_engine_;
    std::string docker_image_;
    
    void detect_container_engine();
    bool run_compilation_pipeline(const std::string& source_filename);
    bool execute_command(const std::string& cmd);
    std::string build_docker_command(const std::string& inner_cmd);
};