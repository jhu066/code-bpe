#include <unordered_map>
#include <nlohmann/json.hpp>
namespace bpe {

    struct pair_hash {
        template <class T1, class T2> std::size_t operator()(const std::pair<T1, T2> &p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            size_t seed = h1;
            return h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    };    

    using idpair = std::pair<int, int>;
    using pc = std::unordered_map<idpair, int, pair_hash>;
    using nlohmann::json;

    class cBPEencoder {        
        public:
            int max_freq;
            
            // store intermediate result; "a;b;" -> "a-b;c" -> "a-b-c"
            std::vector<std::string> learn_buffer; 
            std::unordered_map<std::string, uint32_t> pattern_count; 
            // learning product 
            std::unordered_map<std::string, uint32_t> token_to_int; 
            std::unordered_map<std::string, uint32_t> decoder; 
            std::vector<std::string> int_to_token;
            // last stored vocab size
            size_t vocab_size_last;
            json vocab_json; 

        cBPEencoder(int max_freq);
        ~cBPEencoder();
        
        void train();
        std::string pctrace_to_idtrace(std::string pctrace, bool add_to_buffer);
        void debug_print();
        std::vector<std::string> get_train_set();
        int get_vocab_size();
        void save_model();
        void test_trainset();



        //     std::vector<std::vector<int>> trainbuffer;
        //     std::vector<std::vector<std::string>> newbatch;
        //     std::vector<std::string> int_to_token;
        //     // // pair -> vocabid
        //     pc merge_pattern; 
        //     void count_patterns(pc &pair_counts);
        //     std::vector<std::string> get_train_set();

        
        // std::vector<int> infer(std::vector<std::string> trace_tmp, bool add_train_buffer);
        // std::vector<int> infer_string(std::string trace_tmp, bool add_train_buffer);
        // std::vector<int> load_trace(std::vector<std::string> trace_tmp, bool add_train_buffer);
        // 
        // void learn();

        // // helper section 
        // std::vector<int> tokens_to_ids(std::vector<std::string> trace_tmp, bool add_train_buffer, int* newpc);
        // void dict_print();
        // void rinse_buffer();
        // 
        // int get_vocab_size();

    };

}