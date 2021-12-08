from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

cdef extern from "bpe.h" namespace "bpe":
    cdef cppclass cBPEencoder:
        cBPEencoder(int) except +
        # variable
        vector[string] learn_buffer
        # func
        void train()
        void train1()
        string pctrace_to_idtrace(string, bool)
        void debug_print()
        vector[string] get_train_set()
        int get_vocab_size()

cdef class pyBPEencoder:
    cdef cBPEencoder *encoder
    
    def __cinit__(self, int max_freq):
        self.encoder = new cBPEencoder(max_freq)
    
    def debug_print(self):
        self.encoder.debug_print()
    
    
    
    # construction zone below    
    def train(self, trainbatch):
        # trainbatch to be a list of string pctrace 
        for trace in trainbatch:
            self.encoder.learn_buffer.push_back(trace.encode('utf-8'))
        self.encoder.train1()

    def infer(self, trace):
        cdef string ctrace = trace.encode('utf-8')
        cdef string res = self.encoder.pctrace_to_idtrace(ctrace, False) 
        print (res)
    
    def get_train_set(self):
    #TODO:
        pass 
        
    def get_vocab_size(self):
        return self.encoder.get_vocab_size()

"""
cdef extern from "bpe.h" namespace "bpe":
    cdef cppclass cBPEencoder:
        cBPEencoder(int) except +
        vector[vector[string]] newbatch
        void train()
        vector[int] infer(vector[string], bool)
        vector[int] infer_string(string, bool)
        void dict_print()
        vector[string] get_train_set()
        int get_vocab_size()

cdef class pyBPEencoder:
    cdef cBPEencoder *encoder
    
    def __cinit__(self, int max_freq):
        self.encoder = new cBPEencoder(max_freq)

    def train(self, trainbatch):
        cdef vector[string] trace_c_tmp
        for trace in trainbatch:
            trace_c_tmp.clear()
            for i in range(len(trace)):
                trace_c_tmp.push_back(trace[i].encode('utf-8'))
            self.encoder.newbatch.push_back(trace_c_tmp)

        self.encoder.train()
    
    def infer(self, trace, add_buffer, trace_str=None):
        cdef vector[string] trace_c_tmp
        cdef vector[int] res
        cdef bool add_train_buffer = False
        
        res = self.encoder.infer_string(trace_str.encode('utf-8'), add_train_buffer)
        
        #for i in range(len(trace)):
        #    trace_c_tmp.push_back(trace[i].encode('utf-8'))
        #if add_buffer:
        #    add_train_buffer = True
        #res = self.encoder.infer(trace_c_tmp, add_train_buffer)
        
        pyres = [i for i in res]
        return pyres[0], pyres[1:]

    def dict_print(self):
        self.encoder.dict_print()

    def get_train_set(self):
        cdef vector[string] res
        res = self.encoder.get_train_set()
        pyres = []
        for trace in res:
            trace = trace.decode()
            tmp = [int(i) for i in trace.split(";") if i]
            pyres.append(tmp)
        return pyres
    
    def get_vocab_size(self):
        return self.encoder.get_vocab_size()
                
"""