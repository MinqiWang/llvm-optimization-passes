#include "dfa/framework.h"

class Expression
{
private:
	unsigned _opcode; const Value * _lhs, * _rhs;
public:
	Expression(unsigned opcode = 0, 
		   const Value * lhs = nullptr, 
		   const Value * rhs = nullptr) : 
		_opcode(opcode), _lhs(lhs), _rhs(rhs) {}

	bool operator==(const Expression & Expr) const
	{
		bool isOpEqual = _opcode == Expr.getOpcode();
		bool areOperandsEqualCase1 = (_lhs == Expr.getLHSOperand() && _rhs == Expr.getRHSOperand());
		bool areOperandsEqualCase2 = (Instruction::isCommutative(_opcode) && _lhs == Expr.getRHSOperand() && _rhs == Expr.getLHSOperand());
		return isOpEqual && (areOperandsEqualCase1 || areOperandsEqualCase2);
	}

	unsigned getOpcode() const { return _opcode; }
	const Value * getLHSOperand() const { return _lhs; }
	const Value * getRHSOperand() const { return _rhs; }

	friend raw_ostream & operator<<(raw_ostream & outs, const Expression & Expr);
}; 

raw_ostream & operator<<(raw_ostream & outs, const Expression & Expr)
{
	outs << "[" << Instruction::getOpcodeName(Expr._opcode) << " ";
		Expr.getLHSOperand() ->printAsOperand(outs, false); outs << ", ";
		Expr.getRHSOperand() ->printAsOperand(outs, false);	outs << "]";

	return outs;
}

namespace std {
// Construct a hash code for 'Expression'.
template <>
struct hash < Expression >
{
	std::size_t operator()(const Expression & expr) const
	{
		std::hash < unsigned > unsigned_hasher; std::hash < const Value * > value_ptr_hasher;

		std::size_t opcode_hash = unsigned_hasher(expr.getOpcode());
		std::size_t lhs_operand_hash = value_ptr_hasher((expr.getLHSOperand()));
		std::size_t rhs_operand_hash = value_ptr_hasher((expr.getRHSOperand()));

		return opcode_hash ^ (lhs_operand_hash << 1) ^ (rhs_operand_hash << 1);
	}
};
} // namespace std

namespace {
class AvailExpr : public dfa::Framework < Expression, 
	                                  dfa::Direction::Forward >
{
protected:


	virtual void _initializeDomain(const Function & func) override final
	{
		for (auto & bb : func)
		{
			for (auto & inst : bb)
			{
				if (inst.isBinaryOp()) // We only consider bnary ops. Note that phi is not a binary op
				{
					int opcode = inst.getOpcode();
					auto opIter = inst.op_begin();
					Value * _1st_operand = *(opIter);
					Value * _2nd_operand = *(++opIter);
					_domain.insert(Expression(opcode, _1st_operand, _2nd_operand)); // Move insert
				}
			}
		}
	}
	virtual BitVector __getBoundaryCondition(const Function & func,
	                                         const BasicBlock & bb) const override final
	{
		BitVector * bc = new BitVector(_domain.size(), false); // All zeros, i.e. empty set
		return *bc;
	}
	virtual void _initializeInstBVMap(const Function & func) override final
	{
		for (auto & bb : func)
		{
			for (auto & inst : bb)
			{
				_inst_bv_map.insert(std::make_pair(&inst, 
					                           BitVector(_domain.size(), true))); // All full sets
			}
		}
	}
	virtual BitVector __meetOp(const BasicBlock & bb) override final
	{
		BitVector * result = new BitVector(_domain.size(), true);
		int numOfPreds = 0;
		for (const BasicBlock * Pred : predecessors(&bb))
		{
			auto instIter = Pred->end();
			const Instruction & lastInst = *(--instIter);
			BitVector & bv = _inst_bv_map.at(&lastInst);
			*result &= bv; // Meet op is a big "and"
			numOfPreds++;
		}
		if (numOfPreds == 0) // Entry block
		{
			delete result;
			result = new BitVector(_domain.size(), false); // Initial condition -- Empty set
		}
		return *result;
	}
	virtual bool __instTransferFunc(const Instruction & inst, 
	                                const BitVector & ibv, BitVector & obv) override final
	{
		bool result = false;
		BitVector newbv = ibv;
		if (inst.isBinaryOp())
		{
			// Because of SSA, there is no re-definition of any variable so there will be no KILL set
			// Simply record this expression as available
			int opcode = inst.getOpcode();
			auto opIter = inst.op_begin();
			Value * _1st_operand = *(opIter);
			Value * _2nd_operand = *(++opIter);
			unsigned bv_idx = 0;
			for (auto & element : _domain)
			{
				if (Expression(opcode, _1st_operand, _2nd_operand) == element)
				{
					newbv[bv_idx] = true;
				}
				bv_idx++;
			}
		}
		if (obv != newbv)
		{
			result = true;
			obv = newbv;
		}
		return result;
	}
public:
	static char ID;

	AvailExpr() : dfa::Framework < Expression, 
		                       dfa::Direction::Forward > (ID) {}
};

char AvailExpr::ID = 1; 
RegisterPass < AvailExpr > Y ("avail_expr", "Available Expression");
} // namespace anonymous