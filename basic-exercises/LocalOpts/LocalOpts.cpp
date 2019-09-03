#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {
class LocalOpt : public ModulePass
{
/*
* Local Optimizations:
* 
* Algebraic Identity:
*	x+0 = 0+x => x
*	x*1 = 1*x => x
* Strength Reductions:
*	2*x = x*2 => x<<1
*	x/2 => x>>1 (only deal with unsigned x because -1 >> 1 (which evaluates to -1) is not equivalent to -1 / 2 (which evaluates to 0))
* Multi-Inst Optimization:
*	a = b+1 (or 1+b), c = a-1 (or -1 + a or a + (-1)) => a = b+1 (or 1+b), c = b
*	a = b*3 (or 3*b), c = a/3 => a = b*3 (or 3*b), c = b
*/
private:
	void multiInstCase1(Instruction & I, Value * toReplace, int * numOfOpts) // Multi-Inst a = b + 1 or a = 1 + b
	{
		for (auto iter = I.user_begin(); iter != I.user_end(); ++iter)
		{
			Instruction & user = *(dyn_cast < Instruction > (*iter));
			if (!(user.isBinaryOp())) // Only consider binary ops
			{
				return;
			}
			auto opIter = user.op_begin();
			Value * _1st_operand = *(opIter);
			Value * _2nd_operand = *(++opIter);
			if (!(_1st_operand->getType()->isIntegerTy() && _2nd_operand->getType()->isIntegerTy()))
			{
				return; // Both the operands must be integer
			}
			if (user.getOpcode() == 13) // sub
			{
				if (ConstantInt * c2 = dyn_cast < ConstantInt > (_2nd_operand))
				{
					if (c2->isOne()) // c = a - 1
					{
						outs() << "Multi-Inst case a - 1: " << user << "\n";
						user.replaceAllUsesWith(toReplace);
						user.eraseFromParent();
						(*numOfOpts)++;
					}
				}
			}
			else if (user.getOpcode() == 11) // add
			{
				if (ConstantInt * c1 = dyn_cast < ConstantInt > (_1st_operand))
				{
					if (c1->isMinusOne()) // c = -1 + a
					{
						outs() << "Multi-Inst case -1 + a: " << user << "\n";
						user.replaceAllUsesWith(toReplace);
						user.eraseFromParent();
						(*numOfOpts)++;
					}
				}
				else if (ConstantInt * c2 = dyn_cast < ConstantInt > (_2nd_operand))
				{
					if (c2->isMinusOne()) // c = a + (-1)
					{
						outs() << "Multi-Inst case a + (-1): " << user << "\n";
						user.replaceAllUsesWith(toReplace);
						user.eraseFromParent();
						(*numOfOpts)++;
					}
				}
			}
		}
	}

	void multiInstCase2(Instruction & I, Value * toReplace, int * numOfOpts) // Multi-Inst a = b * 3 or a = 3 * b
	{
		for (auto iter = I.user_begin(); iter != I.user_end(); ++iter)
		{
			Instruction & user = *(dyn_cast < Instruction > (*iter));
			if (!(user.isBinaryOp())) // Only consider binary ops
			{
				return;
			}
			auto opIter = user.op_begin();
			Value * _1st_operand = *(opIter);
			Value * _2nd_operand = *(++opIter);
			if (!(_1st_operand->getType()->isIntegerTy() && _2nd_operand->getType()->isIntegerTy()))
			{
				return; // Both the operands must be integer
			}
			if (user.getOpcode() == 17 || user.getOpcode() == 18) // udiv or sdiv
			{
				if (ConstantInt * c2 = dyn_cast < ConstantInt > (_2nd_operand))
				{
					if (c2->equalsInt(3)) // c = a / 3 where a can be either signed or unsigned
					{
						outs() << "Multi-Inst case a / 3: " << user << "\n";
						user.replaceAllUsesWith(toReplace);
						user.eraseFromParent();
						(*numOfOpts)++;
					}
				}
			}
		}
	}

	int * runOnBasicBlock(BasicBlock & B)
	{
		outs() << "\n";
		int numOfOpts = 0;
		int numOfOptsAI = 0; // Number of Algebraic Identity optimizations
		int numOfOptsSR = 0; // Number of Strength Reductions optimizations
		int numOfOptsMI = 0; // Number of Multi-Inst optimizations
		int * result = new int[4]();

		for (auto instIter = B.begin(); instIter != B.end(); ++instIter)
		{
			Instruction & inst = *instIter;
			if (inst.isBinaryOp()) // skip all non-binary operations because those are out of our interest
			{
				outs() << "New Inst: " << inst << "\n";
				auto opIter = inst.op_begin();
				Value * _1st_operand = *(opIter);
				Value * _2nd_operand = *(++opIter);
				if (!(_1st_operand->getType()->isIntegerTy() && _2nd_operand->getType()->isIntegerTy()))
				{
					return 0; // Both the operands must be integer
				}

				if (inst.getOpcode() == 11) // add
				{
					if (ConstantInt * c1 = dyn_cast < ConstantInt > (_1st_operand))
					{
						if (c1->isZero()) // 0 + x
						{
							outs() << "Algebraic Identity case 0 + x: " << inst << "\n";
							numOfOptsAI++;
							numOfOpts++;
							inst.replaceAllUsesWith(_2nd_operand);
							instIter = inst.eraseFromParent();
							--instIter;
						}
						else if (c1->isOne()) // Multi-Inst a = 1 + b
						{
							outs() << "(Potential) Multi-Inst case a = 1 + b: " << inst << "\n";
							int subnum = 0;
							multiInstCase1(inst, _2nd_operand, &subnum);
							numOfOptsMI += subnum;
							numOfOpts += subnum;
						}
					}
					else if (ConstantInt * c2 = dyn_cast < ConstantInt > (_2nd_operand))
					{
						if (c2->isZero()) // x + 0
						{
							outs() << "Algebraic Identity case x + 0: " << inst << "\n";
							numOfOptsAI++;
							numOfOpts++;
							inst.replaceAllUsesWith(_1st_operand);
							instIter = inst.eraseFromParent();
							--instIter;
						}
						else if (c2->isOne()) // Multi-Inst a = b + 1
						{
							outs() << "(Potential) Multi-Inst case a = b + 1: " << inst << "\n";
							int subnum = 0;
							multiInstCase1(inst, _1st_operand, &subnum);
							numOfOptsMI += subnum;
							numOfOpts += subnum;
						}
					}
				}
				else if (inst.getOpcode() == 15) // mul
				{
					if (ConstantInt * c1 = dyn_cast < ConstantInt > (_1st_operand))
					{
						if (c1->isOne()) // 1 * x
						{
							outs() << "Algebraic Identity case 1 * x: " << inst << "\n";
							numOfOptsAI++;
							numOfOpts++;
							inst.replaceAllUsesWith(_2nd_operand);
							instIter = inst.eraseFromParent();
							--instIter;
						}
						else if (c1->equalsInt(2)) // 2 * x
						{
							outs() << "Strength Reductions case 2 * x: " << inst << "\n";
							numOfOptsSR++;
							numOfOpts++;
							Instruction * new_inst = BinaryOperator::Create(Instruction::Shl, _2nd_operand, ConstantInt::get(B.getContext(), llvm::APInt(32, 1, false)));
							new_inst->insertAfter(&inst);
							inst.replaceAllUsesWith(new_inst);
							instIter = inst.eraseFromParent();
							--instIter;
						}
						else if (c1->equalsInt(3)) // Multi-Inst a = 3 * b
						{
							outs() << "(Potential) Multi-Inst case a = 3 * b: " << inst << "\n";
							int subnum = 0;
							multiInstCase2(inst, _2nd_operand, &subnum);
							numOfOptsMI += subnum;
							numOfOpts += subnum;
						}
					}
					else if (ConstantInt * c2 = dyn_cast < ConstantInt > (_2nd_operand))
					{
						if (c2->isOne()) // x * 1
						{
							outs() << "Algebraic Identity case x * 1: " << inst << "\n";
							numOfOptsAI++;
							numOfOpts++;
							inst.replaceAllUsesWith(_1st_operand);
							instIter = inst.eraseFromParent();
							--instIter;
						}
						else if (c2->equalsInt(2)) // x * 2
						{
							outs() << "Strength Reductions case x * 2: " << inst << "\n";
							numOfOptsSR++;
							numOfOpts++;
							Instruction * new_inst = BinaryOperator::Create(Instruction::Shl, _1st_operand, ConstantInt::get(B.getContext(), llvm::APInt(32, 1, false)));
							new_inst->insertAfter(&inst);
							inst.replaceAllUsesWith(new_inst);
							instIter = inst.eraseFromParent();
							--instIter;
						}
						else if (c2->equalsInt(3)) // Multi-Inst a = b * 3
						{
							outs() << "(Potential) Multi-Inst case a = b * 3: " << inst << "\n";
							int subnum = 0;
							multiInstCase2(inst, _1st_operand, &subnum);
							numOfOptsMI += subnum;
							numOfOpts += subnum;
						}
					}

				}
				else if (inst.getOpcode() == 17) // udiv
				{
					if (ConstantInt * c2 = dyn_cast < ConstantInt > (_2nd_operand))
					{
						if (c2->equalsInt(2)) // x / 2 where x is unsigned
						{
							outs() << "Strength Reductions case x / 2 (unsigned x): " << inst << "\n";
							numOfOptsSR++;
							numOfOpts++;
							Instruction * new_inst = BinaryOperator::Create(Instruction::AShr, _1st_operand, ConstantInt::get(B.getContext(), llvm::APInt(32, 1, false)));
							new_inst->insertAfter(&inst);
							inst.replaceAllUsesWith(new_inst);
							instIter = inst.eraseFromParent();
							--instIter;
						}
					}
				}
			}
		}
		result[0] = numOfOpts;
		result[1] = numOfOptsAI;
		result[2] = numOfOptsSR;
		result[3] = numOfOptsMI;
		return result;
	}
	
	int * runOnFunction(Function & F)
	{	
		outs() << "\n===================================================================================\n"; // Eye catcher
		outs() << "***** Function: " << F.getName() << "\n";
		outs() << "===================================================================================\n"; // Eye catcher

		int * result = new int[4]();

		for (auto iter = F.begin(); iter != F.end(); ++iter)
		{
			int * subresult = runOnBasicBlock(*iter);
			result[0] += subresult[0];
			result[1] += subresult[1];
			result[2] += subresult[2];
			result[3] += subresult[3];
			delete[] subresult;
		}
		return result;
	}
	
public:
	static char ID;

	LocalOpt() : ModulePass(ID)
	{}

	~LocalOpt()
	{}

	virtual void getAnalysisUsage(AnalysisUsage & AU) const
	{
		AU.setPreservesCFG();
	}
  
	virtual bool runOnModule(Module & M)
	{
		outs() << "\n[ CSCD70 Local Optimization Pass ]\n\n";
		outs() << "Test Module: " << M.getName() << "\n";

		int numOfOpts = 0;
		int numOfOptsAI = 0; // Number of Algebraic Identity optimizations
		int numOfOptsSR = 0; // Number of Strength Reductions optimizations
		int numOfOptsMI = 0; // Number of Multi-Inst optimizations

		for (auto iter = M.begin(); iter != M.end(); ++iter)
		{
			int * subresult = runOnFunction(*iter);
			numOfOpts += subresult[0];
			numOfOptsAI += subresult[1];
			numOfOptsSR += subresult[2];
			numOfOptsMI += subresult[3];
			delete[] subresult;
		}
		outs() << "\n-----------------------------------------------------------------------------------\n"; // Eye catcher
		outs() << "Summary --\n";
		outs() << "Transformations applied:\n";
		outs() << "    Algebraic Identity: " << numOfOptsAI << "\n";
		outs() << "    Strength Reduction: " << numOfOptsSR << "\n";
		outs() << "    Multi-Inst Optimization: " << numOfOptsMI << "\n";
		outs() << "    Total number of optimizations: " << numOfOpts << "\n";

		return (numOfOpts != 0);
	}
};

char LocalOpt::ID = 0;

RegisterPass < LocalOpt > X ("local-opt", "Some basic local optimizations."); 

} // Anonymous Namespace