#ifndef KUMIRCODEGENERATOR_GENERATOR_H
#define KUMIRCODEGENERATOR_GENERATOR_H

#include <QtCore>

#include "dataformats/ast.h"
#include "dataformats/ast_variable.h"
#include "dataformats/ast_algorhitm.h"
#include "dataformats/lexem.h"
#include "errormessages/errormessages.h"
#include "interfaces/generatorinterface.h"
#include "vm/vm_enums.h"
#include "vm/vm_instruction.hpp"



namespace Bytecode {
class Data;
}

namespace KumirCodeGenerator {

typedef Shared::GeneratorInterface::DebugLevel DebugLevel;
struct ConstValue {
    QVariant value;
    Bytecode::ValueType baseType;
    quint8 dimension;
    inline bool operator==(const ConstValue & other) {
        return
                baseType == other.baseType &&
                dimension == other.dimension &&
                value == other.value;
    }
    inline ConstValue() {
        baseType = Bytecode::VT_void;
        dimension = 0;
    }
};

class Generator : public QObject
{

    Q_OBJECT
public:
    explicit Generator(QObject *parent = 0);
    void reset(const AST::Data * ast, Bytecode::Data * bc, DebugLevel debugLevel);
    void addModule(const AST::Module * mod);
    void generateConstantTable();
    void generateExternTable();
private:
    quint16 constantValue(Bytecode::ValueType type, quint8 dimension, const QVariant & value);
    void addKumirModule(int id, const AST::Module * mod);
    void addFunction(int id, int moduleId, Bytecode::ElemType type, const AST::Algorhitm * alg);
    void addInputArgumentsMainAlgorhitm(int moduleId, int algorhitmId, const AST::Module * mod, const AST::Algorhitm * alg);

    QList<Bytecode::Instruction> instructions(
        int modId, int algId, int level,
        const QList<AST::Statement*> & statements);

    static void shiftInstructions(QList<Bytecode::Instruction> &instrs, int offset);
    static void setBreakAddress(QList<Bytecode::Instruction> &instrs, int level, int address);

    void ERRORR(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void ASSIGN(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void ASSERT(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void PAUSE_STOP(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void INIT(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void CALL_SPECIAL(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void LOOP(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void IFTHENELSE(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void SWITCHCASEELSE(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);
    void BREAK(int modId, int algId, int level, const AST::Statement * st, QList<Bytecode::Instruction> & result);

    QList<Bytecode::Instruction> calculate(int modId, int algId, int level, const AST::Expression* st);

    void findVariable(int modId, int algId, const AST::Variable * var, Bytecode::VariableScope & scope, quint16 & id) const;
    static const AST::Variable * returnValue(const AST::Algorhitm * alg);
    void findFunction(const AST::Algorhitm * alg, quint8 & module, quint16 & id) ;


    static QPair<Bytecode::ValueType, size_t> valueType(const AST::Type & t);
    static Bytecode::ValueKind valueKind(AST::VariableAccessType t);
    static Bytecode::InstructionType operation(AST::ExpressionOperator op);

    const AST::Data * m_ast;
    Bytecode::Data * m_bc;
    QList< ConstValue > l_constants;
    QList< QPair<quint8,quint16> > l_externs;
    DebugLevel e_debugLevel;

};

} // namespace KumirCodeGenerator

#endif // KUMIRCODEGENERATOR_GENERATOR_H
