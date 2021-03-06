#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <utility>
#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include "bpe.h"
// using nlohmann::json;

//using 
namespace bpe {
    using namespace std;
    
    
    void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr) {
        size_t pos = data.find(toSearch);
        while( pos != std::string::npos)
        {
            data.replace(pos, toSearch.size(), replaceStr);
            pos = data.find(toSearch);
        }
    }

    void join_list(vector<int> id_trace, std::string & data, std::string delim) {
        data += delim;
        for (int const &i: id_trace) {
            data += to_string(i);
            data += delim;
        }
    }

    void join_string(vector<string> pc_trace, std::string & data, std::string delim) {
        data += delim;
        for (auto i: pc_trace) {
            data += i;
            data += delim;
        }
    }

    cBPEencoder::cBPEencoder(int _max_freq) {
        max_freq = _max_freq;
        cout << "max_freq assigned: " << max_freq << endl;
        token_to_int["<|endoftext|>"] = 0;
        int_to_token.push_back("<|endoftext|>");
    }

    cBPEencoder::~cBPEencoder() {
    }
    
    int cBPEencoder::get_vocab_size() {
        return token_to_int.size();
    }

    void cBPEencoder::save_model() {
        int i; 
        std::ofstream otest("./trained_encoder/vocab.json");
        json j1;
        // for (i = 0; i < int_to_token.size(); i++) {
        //     j1[int_to_token[i].c_str()] = i;
        // } 
        for (auto t: pair_to_id) {
            cout << t.first << endl;
            return;
        }
        otest << std::setw(4) << j1 << std::endl;
    }
    
    void cBPEencoder::train1() {
        std::chrono::steady_clock::time_point t1, t2, t3;
        vector<size_t> delim_positions;
        size_t pos, n_pos;
        int lpair, rpair, i, tmp_rpair;
        bool second;
        string word1, word2; 
        t1 = std::chrono::steady_clock::now();
        // parsing into buffer and pair count 
        for (auto pctrace: learn_buffer) {  
            delim_positions.clear(); 
            pos = 0;
            while(pos != string::npos) {
                delim_positions.push_back(pos);
                pos = pctrace.find(";", pos + 1 );
            }
            second = false;
            for (i = 0; i < delim_positions.size(); i++) {
                word1 = pctrace.substr(delim_positions[i], delim_positions[i+1] - delim_positions[i]);
                auto it = token_to_int.find(word1);
                // see if new vocab 
                if (it == token_to_int.end()) {
                    int_to_token.push_back(word1);
                    token_to_int[word1] = int_to_token.size() - 1;
                } 
                if (second) {
                    lpair = rpair;
                    rpair = token_to_int[word1];
                    auto newpair = make_pair(lpair, rpair);
                    traces_buffer.push_back(newpair);
                    auto findpair = pair_count.find(newpair);
                    if (findpair == pair_count.end()) {
                        pair_count[newpair] = 1;
                    } else {
                        findpair->second += 1;
                    }
                } else {
                    rpair = token_to_int[word1];
                    second = true;
                }             
            }
            
        }
        cout << "total pairs in buffer: " <<  traces_buffer.size() << endl;
        cout << "uniq pairs with count: " <<  pair_count.size() << endl;
        t2 = std::chrono::steady_clock::now();
        std::cout << "Time took for parsing = " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6 << "[s]" << std::endl;

        // return;
        
        // find the replace the freq pattern in one pass 
        int newid;
        string tmppush;
        for (auto pi: pair_count) {
            if (pi.second > max_freq) { // new pattern to merge 
                auto findpair = pair_to_id.find(pi.first);
                if (findpair == pair_to_id.end()) {
                    // tmppush = (pi.first).first + " " + (pi.first).second;
                    // cout << (pi.first).first << " " <<  (pi.first).second;
                    // return;
                    // int_to_token.push_back((pi.first).first + " " + (pi.first).second);

                    // newid = int_to_token.size() - 1;
                    newid = int_to_token.size() + pair_to_id.size();
                    pair_to_id[pi.first] = newid;
                } 
                
                
                int cnt = 0;
                
                for (auto cur=traces_buffer.begin(); cur!=traces_buffer.end(); ) {                                 
                    if (*cur == pi.first) { 
                        cnt += 1; 
                        if (cur != traces_buffer.begin()) {
                            auto prev = cur;
                            prev--;
                            if (prev->second == cur->first) { // make sure not the gap between traces 
                                pair_count[*prev] -= 1; //vist count dec 
                                prev->second = newid;
                                // add new pattern if not exist 
                                auto findpair = pair_count.find(*prev);
                                if (findpair == pair_count.end()) {
                                    pair_count[*prev] = 1;
                                } else {
                                    findpair->second += 1;
                                }
                            }
                        }
                        tmp_rpair = cur->second;
                        cur = traces_buffer.erase(cur); // then point to next 

                        if (cur != traces_buffer.end()) {
                            if (cur->first == tmp_rpair) { // make sure it's not at the trace gap 
                                pair_count[*cur] -= 1; //visit  count dec 
                                cur->first = newid;
                                // add new pattern if not exist 
                                auto findpair = pair_count.find(*cur);
                                if (findpair == pair_count.end()) {
                                    pair_count[*cur] = 1;
                                } else {
                                    findpair->second += 1;
                                }
                            }
                        }
                        
                    } else {
                        cur++;
                    }
                }
                
            }

        }

        cout << "merged buffer " << traces_buffer.size() << endl;
        cout << "unique pattern w/ cnt: " << pair_count.size() << endl;   
        cout << "the end" << endl;

        t3 = std::chrono::steady_clock::now();
        std::cout << "Time took for replacing  = " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1e6 << "[s]" << std::endl;
        save_model();
        return;



    }

// construction zone ----------------------------
    void cBPEencoder::train() {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point t1, t2, t3;
        // 1. get all raw vocab and pattern count 
        string word, wordpair;
        size_t pos, n_pos;
        int i, j, k;
        vector<size_t> delim_positions;
        int merged = 0;
        int cnt = 3;

        while (1) {
            pattern_count.clear();
            t1 = std::chrono::steady_clock::now();
            // 1. acquire all vocab & pattern 
            for (auto pctrace: learn_buffer) {    
                delim_positions.clear();    
                pos = 0;
                while(pos != string::npos)
                {
                    delim_positions.push_back(pos);
                    pos = pctrace.find(";", pos + 1 );
                }
                
                for(i = 0; i < delim_positions.size()-2; i++) {
                    word = pctrace.substr(delim_positions[i], delim_positions[i+1] - delim_positions[i]);       // ;0x908
                    wordpair = pctrace.substr(delim_positions[i], delim_positions[i+2] - delim_positions[i]);   
                    // ;0x908;0x293 => ;0x908-0x293

                    // update vocab dict
                    if (token_to_int.find(word) == token_to_int.end()) {
                        int_to_token.push_back(word);
                        token_to_int[word] = int_to_token.size() - 1;
                    }
                    // update pattern count dict 
                    auto it = pattern_count.find(wordpair);
                    if (it == pattern_count.end()) {  
                        pattern_count[wordpair] = 1;
                    } else {
                        // pattern_count[wordpair] += 1; // TODO: 
                        it->second += 1;
                    }
                }
                
            }
            
            t2 = std::chrono::steady_clock::now();
            std::cout << "Time took for parsing = " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6 << "[s]" << std::endl;
            
            merged = 0;
            int maxcount = 0;
            vector<string> related;
            for (auto t: pattern_count) {
                if (t.second > max_freq) { 
                    string tmp (t.first.substr(1));
                    pos = tmp.find(";", 1);                    
                    if (find(related.begin(), related.end(), tmp.substr(0, pos)) != related.end()) {
                        continue;
                    }
                    n_pos = tmp.find(";", pos+1);
                    if (find(related.begin(), related.end(), tmp.substr(pos+1, n_pos-pos)) != related.end()) {
                        continue;
                    }
                    related.push_back(tmp.substr(0, pos));
                    related.push_back(tmp.substr(pos+1, n_pos-pos));
                    string replacement (t.first); 
                    replacement.replace(replacement.find(";", 1), 1, "-"); // replacement; 0x908-0x897
                    merged += 1;
                    // 100 traces 
                    for (auto &trace: learn_buffer) {                        
                        if (trace.find(t.first) != string::npos) {                            
                            findAndReplaceAll(trace, t.first, replacement); 
                        }              
                    }                    
                }            
            }
            t3 = std::chrono::steady_clock::now();
            std::cout << "Time took for replacing = " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1e6 << "[s]" << std::endl;
            
            cout << "round end, vocab size is: " << token_to_int.size() 
            << "  merged pattern " << merged << endl;
            
            if (merged == 0) break;
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1e6 << "[s]" << std::endl;
        save_model();

    }

    std::string cBPEencoder::pctrace_to_idtrace(std::string pctrace, bool add_to_buffer) {
        cout << pctrace << endl;

        string res = "";
        return res;

    }
    void cBPEencoder::debug_print() {
        // return;
        int i; 
        cout << "\n[HJ]:Print the vocabs, < token: id >: " << endl;
        for (i = 0; i < int_to_token.size(); i++) {
            printf("\t< %d: %s >\n", i, int_to_token[i].c_str());
        } 

        // cout << "\n[HJ]:Print the patterncount, < pair: count >: " << endl;
        // for (auto t: pattern_count) {
        //     cout << "pattern: " << t.first << "; count: " << t.second << endl;            
        // }
        save_model();
    }

    // std::vector<std::string> cBPEencoder::get_train_set() {
    void cBPEencoder::test_trainset() {
        vector<size_t> delim_positions;
        vector<int> res;
        size_t pos;
        string word;
        int i;
        for (auto pctrace: learn_buffer) {
            // cout << pctrace << endl;
            delim_positions.clear();    
            pos = 0;
            cout << "trace into: " ;
            while(pos != string::npos) {
                delim_positions.push_back(pos);
                pos = pctrace.find(";", pos + 1 );
            }
            for(i = 0; i < delim_positions.size()-1; i++) {
                word = pctrace.substr(delim_positions[i], delim_positions[i+1] - delim_positions[i]);
                cout << token_to_int[word] << " " ;
            }
            cout << endl;
        }

    }
    // construction zone



    // void cBPEencoder::train() {
    //     cout << "into trianing" << endl;
    //     for (auto trace : newbatch) {
    //         load_trace(trace, true);
    //     }   
    //     newbatch.clear();
    //     cout << "loaded" << endl;

    //     int merged_in = 0;
        
    //     pc pair_counts;
    //     while (true) {
    //         merged_in = 0;
    //         pair_counts.clear();
    //         count_patterns(pair_counts);

    //         int patternid;
    //         string tmppattern, pairfirst, pairsecond; 

    //         // int maxc = 0;
    //         // idpair maxp;
    //         // for (auto p: pair_counts) {
    //         //     if (p.second > max_freq && p.second > maxc) {
    //         //         maxp = p.first;
    //         //         maxc = p.second;                    
    //         //     }
    //         // }
    //         // if (!maxc) break;

    //         // tmppattern = int_to_token[maxp.first] + ";" + int_to_token[maxp.second];                   
    //         // int_to_token.push_back(tmppattern);
    //         // token_to_int[tmppattern] = int_to_token.size() - 1;
    //         // merge_pattern[make_pair(maxp.first, maxp.second)] = token_to_int[tmppattern];   
            
    //         for (auto pi: pair_counts) {
    //             if (pi.second > max_freq) {
    //                 tmppattern = int_to_token[(pi.first).first] + ";" + int_to_token[(pi.first).second];
    //                 int_to_token.push_back(tmppattern);
    //                 token_to_int[tmppattern] = int_to_token.size() - 1;
    //                 merge_pattern[make_pair((pi.first).first, (pi.first).second)] = token_to_int[tmppattern];   
    //                 merged_in +=1;
    //             }
    //         }   
    //         if (!merged_in) break;
    //         // rinse for next round
    //         rinse_buffer();
    //     }
    //     save_model();
    // }
    

    // std::vector<int> cBPEencoder::load_trace(std::vector<std::string> trace_tmp, bool add_train_buffer) {
    //     std::vector<int> res;
    //     int newpc; 
    //     std::vector<int> id_trace = tokens_to_ids(trace_tmp, add_train_buffer, &newpc);
        
    //     string tracestring, patternstring, pidstring; 
    //     join_list(id_trace, tracestring, ";");
    //     int before;
    //     while (true){
    //         before = tracestring.size();
    //         for (auto pattern: merge_pattern) {
    //             patternstring = ";" + to_string((pattern.first).first) + ";" + to_string((pattern.first).second) + ";";
    //             pidstring = ";" + to_string(pattern.second) + ";";
    //             findAndReplaceAll(tracestring, patternstring, pidstring);
    //         }
    //         if (tracestring.size() == before) break; 
    //     }

    //     stringstream check1(tracestring);  
    //     string intermediate;
    //     while(getline(check1, intermediate, ';'))
    //     {   
    //         if (intermediate.size()) {
    //             res.push_back(stoi(intermediate));
    //         }
    //     }
    //     return res;
    // }   

    

    // std::vector<int> cBPEencoder::infer(std::vector<std::string> trace_tmp, bool add_train_buffer) {
    //     std::vector<int> res;
    //     int newpc; 
    //     std::vector<int> id_trace = tokens_to_ids(trace_tmp, add_train_buffer, &newpc);
        
    //     string tracestring, patternstring, pidstring; 
    //     join_list(id_trace, tracestring, ";");
    //     int before;
    //     while (true){
    //         before = tracestring.size();
    //         for (auto pattern: merge_pattern) {
    //             patternstring = ";" + to_string((pattern.first).first) + ";" + to_string((pattern.first).second) + ";";
    //             pidstring = ";" + to_string(pattern.second) + ";";
    //             findAndReplaceAll(tracestring, patternstring, pidstring);
    //         }
    //         if (tracestring.size() == before) break; 
    //     }

    //     stringstream check1(tracestring);  
    //     string intermediate;
    //     while(getline(check1, intermediate, ';'))
    //     {   
    //         if (intermediate.size()) {
    //             res.push_back(stoi(intermediate));
    //         }
    //     }
    //     return res;
    //     // std::vector<int> res;
    //     // // scan for new vocab
    //     // int newpc; 
    //     // std::vector<int> id_trace = tokens_to_ids(trace_tmp, add_train_buffer, &newpc);
    //     // string tracestring, patternstring, pidstring; 
    //     // int i;
        
    //     // join_string(trace_tmp, tracestring, ";");
    //     // int before;
    //     // if (true){
    //     //     before = tracestring.size();
    //     //     for (i = int_to_token.size()-1; i >=0; i--) {
    //     //         patternstring = ";" + int_to_token[i] + ";";
    //     //         pidstring = ";" + to_string(i) + ";";
    //     //         findAndReplaceAll(tracestring, patternstring, pidstring);
    //     //     } 

    //     //     // if (tracestring.size() == before) break; // no patterns to merge anymore 
    //     // }

        
    //     // stringstream check1(tracestring);  
    //     // string intermediate;
    //     // res.push_back(newpc);
    //     // while(getline(check1, intermediate, ';'))
    //     // {   
    //     //     if (intermediate.size()) {
    //     //         res.push_back(stoi(intermediate));
    //     //     }
    //     // }
    //     // return res;
    // }

    // std::vector<int> cBPEencoder::infer_string(std::string trace_tmp, bool add_train_buffer) {
    //     std::vector<int> res;
    //     int newpc = 0; // TODO: 
    //     int i;
    //     for (i = int_to_token.size()-1; i >=0; i--) {
    //             findAndReplaceAll(trace_tmp, ";" + int_to_token[i] + ";", ";" + to_string(i) + ";");
    //     } 
    //     stringstream check1(trace_tmp);  
    //     string intermediate;
    //     res.push_back(newpc);
    //     while(getline(check1, intermediate, ';'))
    //     {   
    //         if (intermediate.size()) {
    //             res.push_back(stoi(intermediate));
    //         }
    //     }
    //     return res;
    // } 

    // void cBPEencoder::rinse_buffer() {
    //     std::vector<std::vector<int>> trainbuffer_rinse;
    //     copy(trainbuffer.begin(), trainbuffer.end(), back_inserter(trainbuffer_rinse));
    //     trainbuffer.clear();
    //     string tracestring, patternstring, pidstring; 
    //     std::vector<int> tmpres;
        
    //     for (auto id_trace: trainbuffer_rinse) {   
    //         tracestring = "";
    //         join_list(id_trace, tracestring, ";");
    //         for (auto pattern: merge_pattern) {
    //             patternstring = ";" + to_string((pattern.first).first) + ";" + to_string((pattern.first).second) + ";" ;
    //             pidstring = ";" + to_string(pattern.second) + ";";
    //             findAndReplaceAll(tracestring, patternstring, pidstring);
    //         }            
    //         stringstream check1(tracestring);  
    //         string intermediate;
    //         tmpres.clear();
    //         while(getline(check1, intermediate, ';'))
    //         {
    //             if (intermediate.size()) {
    //                 tmpres.push_back(stoi(intermediate));
    //             }
    //         }
    //         trainbuffer.push_back(tmpres);
    //     }
    // }

    // void cBPEencoder::count_patterns(pc &pair_counts) {
    //     int i;
    //     for (auto id_trace: trainbuffer) {       
    //         bool second = false;
    //         idpair cur_pair;
    //         for (i = 0; i < id_trace.size() -1 ; i++) {
    //             cur_pair = make_pair(id_trace[i], id_trace[i+1]);
    //             auto it = pair_counts.find(cur_pair);
    //             if (it == pair_counts.end()) {
    //                 pair_counts[cur_pair] = 1;
    //             } else {
    //                 pair_counts[cur_pair] += 1;
    //             }
    //         }
    //         // for (int id: id_trace) {
    //         //     if (second) {
    //         //         cur_pair.first = cur_pair.second;
    //         //     }
    //         //     cur_pair.second = id;
    //         //     if (second) {
    //         //         auto it = pair_counts.find(cur_pair);
    //         //         if (it == pair_counts.end()) {
    //         //             pair_counts[cur_pair] = 1;
    //         //         } else {
    //         //             pair_counts[cur_pair] += 1;
    //         //         }
    //         //     }
    //         //     else {
    //         //         second = true;
    //         //     }
    //         // }
    //     }
    // }

    // std::vector<int> cBPEencoder::tokens_to_ids(std::vector<std::string> trace_tmp, bool add_train_buffer, int *newpc) {
    //     std::vector<int> res; 
    //     *newpc = 0;
    //     bool newvocab = add_train_buffer; 
    //     for (auto onepc : trace_tmp) {  
    //         if (token_to_int.count(onepc) == 0) {
    //             newvocab = true;
    //             *newpc += 1;
    //             int_to_token.push_back(onepc);
    //             token_to_int[onepc] = int_to_token.size() - 1;
    //         }    
    //         res.push_back(token_to_int[onepc]);
    //     }
    //     if (newvocab) {
    //         trainbuffer.push_back(res);
    //     }
    //     return res;
    // }

    // void cBPEencoder::dict_print() {
    //     int i; 
    //     cout << "\n[HJ]:Print the int_to_token, < token: id >: " << endl;
    //     for (i = 0; i < int_to_token.size(); i++) {
    //         printf("\t< %s: %d >\n", int_to_token[i].c_str(), token_to_int[int_to_token[i].c_str()]);
    //     } 
    //     cout << "\n[HJ]: token_to_int size: " << token_to_int.size() << endl;
    // }

    // std::vector<std::string> cBPEencoder::get_train_set() {
    //     vector<string> res;
    //     string tracestring;
    //     for (auto id_trace: trainbuffer) {  
    //         tracestring = "";
    //         join_list(id_trace, tracestring, ";");
    //         res.push_back(tracestring);
    //     }
    //     return res;
    // }

    
}
