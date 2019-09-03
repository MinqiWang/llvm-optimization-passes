#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class FunctionInfo : public ModulePass
{
public:
        static char ID;

        FunctionInfo() : ModulePass(ID)
        {}

        ~FunctionInfo()
        {}

        // We don't modify the program, so we preserve all analysis.
        virtual void getAnalysisUsage(AnalysisUsage & AU) const
        {
                AU.setPreservesAll();
        }

        virtual bool runOnModule(Module & M)
        {
                outs() << "CSCD70 Functions Information Pass" << "\n";

                // @TODO
                outs() << "Number of functions: " << M.size() << "\n";
                int numFuncs = M.size();
                StringRef * fNames = new StringRef[numFuncs]();
                int * fArgCounts = new int[numFuncs]();
                int * fCallCounts = new int[numFuncs]();
                int * fBlockCounts = new int[numFuncs]();
                int * fInstCounts = new int[numFuncs]();

                int index = 0;
                for (auto iter = M.begin(); iter != M.end(); ++iter)
                {
                    Function & F = *iter;
                    StringRef name = F.getName();
                    fNames[index] = name;
                    fArgCounts[index] = F.arg_size();
                    fBlockCounts[index] = F.size();
                    int numInsts = 0;
                    for (auto iter2 = F.begin(); iter2 != F.end(); ++iter2){
                        BasicBlock & B = *iter2;
                        numInsts = numInsts + B.size();
                        for (auto iter3 = B.begin(); iter3 != B.end(); ++iter3)
                        {
                            if (CallInst * c = dyn_cast < CallInst > (iter3))
                            {
                            	int calleeIdx = 0;
                            	StringRef callee = c->getCalledFunction()->getName();
                            	for (int i = 0; i < numFuncs; ++i)
                            	{
                            		if (fNames[i].equals(callee))
                            	    {
                            		    calleeIdx = i;
                            		}
                            	}
                            	++fCallCounts[calleeIdx];
                            }
                        }
                    }
                    fInstCounts[index] = numInsts;
                    ++index;
                }

                for (int i = 0; i < numFuncs; ++i)
                {
                    outs() << "===========================================\n"; // Eye catcher
                    outs() << "Function name: " << fNames[i].str() << "\n";
                    outs() << "Number of arguments: " << fArgCounts[i] << "\n";
                    outs() << "Number of calls: " << fCallCounts[i] << "\n";
                    outs() << "Number of basic blocks: " << fBlockCounts[i] << "\n";
                    outs() << "Number of instructions: " << fInstCounts[i] << "\n";
                    outs() << "===========================================\n"; // Eye catcher
                }

                return false;
        }
};

char FunctionInfo::ID = 0;

RegisterPass < FunctionInfo > X ("function-info", "CSCD70: Functions Information");

} // Anonymous Namespace