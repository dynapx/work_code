#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramState.h"

#include <vector>
#include <string>

using namespace clang;        // 使用 clang 命名空间
using namespace ento;         // 使用 ento 命名空间


namespace {
// 定义一个结构体保存函数名和函数指针
struct PyMethodInfo {
    std::string methodName;  // 函数名
    std::string funcName;    // 函数指针对应的函数名
    std::string note;
    int flags=-1;               // 标志，如 METH_VARARGS 等
    int fgs[5]={-1,-1,-1,-1,-1};   
};

// Python 扩展检查器
class PythonExtensionsChecker : public Checker<check::ASTDecl<Decl>> {
public:
    // 用于保存提取的函数名和函数指针
    mutable std::vector<PyMethodInfo> methodInfos;

    // 声明方法，但不实现
    void checkASTDecl(const Decl *D, AnalysisManager &Mgr, BugReporter &BR) const;
};

} // anonymous namespace


//递归函数，用于寻找类型转化下的在函数名称
const Expr* findDeclRefExpr(const Expr* expr)
{
    if(const DeclRefExpr * declRefExpr =dyn_cast<DeclRefExpr>(expr))
    {
        return declRefExpr;

    }
   else if(const CStyleCastExpr* cStyleCastExpr=dyn_cast< CStyleCastExpr>(expr))
    {
        return findDeclRefExpr(cStyleCastExpr->getSubExpr());

    }
   else if(const ImplicitCastExpr* implicitCastExpr=dyn_cast<ImplicitCastExpr>(expr))
    {
        return findDeclRefExpr(implicitCastExpr->getSubExpr());
    }
    else 
    {
        return nullptr;
    }
    
}


// 在类外实现方法
void PythonExtensionsChecker::checkASTDecl(const Decl *D, AnalysisManager &Mgr, BugReporter &BR) const {
    // 确保是全局变量声明
    //llvm::outs()<<"my checker start";
    const VarDecl *varDecl = dyn_cast<VarDecl>(D);
    if (!varDecl || !varDecl->hasGlobalStorage()) {
        return;
    }
    //llvm::outs()<<"point 1\n";
    // 确保是 PyMethodDef 类型
    if (const ArrayType *arrType = varDecl->getType()->getAsArrayTypeUnsafe()) { //这里是判断这个变量定义是不是一个结构体 getAs()将QualType型强制类型转换为RecordType
        //llvm::outs()<<"point 2\n";
        const RecordType *recordType = arrType->getElementType()->getAs<RecordType>();
        if (!recordType) return;
        const RecordDecl *recordDecl = recordType->getDecl(); //getDecl() 方法获取 RecordDecl ,RecordDecl 是一个表示结构体或类的声明对象
        //llvm::outs() << "Record name as string: " << recordDecl->getName().str() << "\n";
         //查找 PyMethodDef 结构体
        if (recordDecl->getNameAsString() == "PyMethodDef") { //这里是IdentifierInfo* ，clang会将相同字符串放在同一地址
            llvm::outs() << "Found PyMethodDef array: " << varDecl->getNameAsString() << "\n"; //这里返回的是结构体数组的名字

            // 遍历数组初始化元素
       
            
            for (const auto &elem : varDecl->getInit()->children()) { //auto表示根据上下文推断的类型，const auto 表示只读，遍历初始化数组  children() 是 Stmt 类（抽象语法树中的语句节点类）的方法，用来返回该语句节点的子节点列表。每个 Stmt 对象可能包含多个子节点。例如，如果一个初始化表达式是一个复合表达式（如数组、结构体、函数调用等），那么这个表达式可能有多个子节点，表示表达式的组成部分。
            
                int n=0;
                PyMethodInfo methodInfo;
                if(const InitListExpr *initListExpr=dyn_cast<InitListExpr>(elem))
                {
                    for(const auto &initElem:initListExpr->children()){
                                           
                        if(const ImplicitCastExpr *castExpr =dyn_cast<ImplicitCastExpr>(initElem)){
                            const Expr *subExpr = castExpr->getSubExpr();
                        
                            const Expr *subExpr2 = subExpr;
                            while(castExpr){
                                while(const CStyleCastExpr *castExpr3 =dyn_cast<CStyleCastExpr>(subExpr2)){
                                    subExpr2 = castExpr3->getSubExpr();
                                    if(const ImplicitCastExpr *castExpr2 =dyn_cast<ImplicitCastExpr>(subExpr2)){
                                        castExpr = castExpr2;
                                        subExpr2 = castExpr2->getSubExpr();
                                        break;
                                    }
                                    continue;
                                }
                                if(const ImplicitCastExpr *castExpr2 =dyn_cast<ImplicitCastExpr>(subExpr2)){
                                    castExpr = castExpr2;
                                    subExpr2 = castExpr2->getSubExpr();
                                }else{
                                    if(const StringLiteral *nameLit =dyn_cast<StringLiteral>(subExpr2)){
                                        if(n==0){
                                            std::string methodName = nameLit->getString().str();//nameLit->getString()返回一个 StringRef，通过.str() 将其转换为一个std::string类型
                                            llvm::outs() << "Method Name: " << methodName << "\n";
                                            methodInfo.methodName = methodName;
                                            n=1;
                                        }else if(n==1){
                                            std::string note = nameLit->getString().str();
                                            llvm::outs() << "note:" << note << "\n";
                                            methodInfo.note = note;
                                            n=0;
                                        }
                                    }
                                    break;
                                }
                                                             	
                            }
                                                        
                                                  
                        }
                        
                           if(const ImplicitCastExpr *castExpr =dyn_cast<ImplicitCastExpr>(initElem)){
                            const Expr *subExpr = castExpr->getSubExpr();
                            if(const CStyleCastExpr *CastExpr=dyn_cast<CStyleCastExpr>(subExpr))
                            {
                                const Expr *subExpr1 =CastExpr->getSubExpr();
                                if (const ImplicitCastExpr *castExpr1=dyn_cast<ImplicitCastExpr>(subExpr1))
                                {
                                    const Expr *subExpr2=castExpr1->getSubExpr();
                                    if(const DeclRefExpr *funcRef = dyn_cast<DeclRefExpr>(subExpr2)){
                                        std::string funcName = funcRef->getNameInfo().getAsString();
                                        llvm::outs() << "Function: " << funcName << "\n";
                                        methodInfo.funcName = funcName;

                            }

                                }

                            }
                            else{

                                if(const DeclRefExpr *funcRef = dyn_cast<DeclRefExpr>(subExpr)){
                                 std::string funcName = funcRef->getNameInfo().getAsString();
                                 llvm::outs() << "Function: " << funcName << "\n";
                                 methodInfo.funcName = funcName;

                            }

                            }
                            
                        }
                        if(const CStyleCastExpr *CScastExpr=dyn_cast<CStyleCastExpr>(initElem))
                        {
                            while(CScastExpr){
                            const Expr *subExpr =CScastExpr->getSubExpr();
                            if(const ImplicitCastExpr *castExpr = dyn_cast<ImplicitCastExpr>(subExpr)){
                                const Expr *subExpr=castExpr->getSubExpr();
                                if(const DeclRefExpr *funcRef =dyn_cast<DeclRefExpr>(subExpr))
                                {
                                    std::string funcName = funcRef->getNameInfo().getAsString();
                                    llvm::outs() << "Function: " << funcName << "\n";
                                    methodInfo.funcName = funcName;
                                }
                                break;
                            }else{
                            	CScastExpr=dyn_cast<CStyleCastExpr>(subExpr);
                            }
                        }

                        }

                       
                        
                        
                        
                        
                        
                        
                        if(auto *binOp = dyn_cast<BinaryOperator>(initElem)){
                            int n=0;                            
                            if(const IntegerLiteral *flagsLit1=dyn_cast<IntegerLiteral>(binOp->getLHS()))
                            {
                                int flags = flagsLit1->getValue().getSExtValue();
                                if(flags)
                                {
                                    llvm::outs() << "Flags: " << flags << "\n";
                                    methodInfo.fgs[n] = flags;
                                    n++;
                                }
                            }else{
                                auto *binOp2 = dyn_cast<BinaryOperator>(binOp->getLHS());
                                
                                if(const IntegerLiteral *flagsLit1=dyn_cast<IntegerLiteral>(binOp2->getLHS()))
                                {
                                    int flags = flagsLit1->getValue().getSExtValue();
                                    if(flags)
                                    {
                                        llvm::outs() << "Flags: " << flags << "\n";
                                        methodInfo.fgs[n] = flags;
                                        n++;
                                    }
                                }else{
                                    auto *binOp3 = dyn_cast<BinaryOperator>(binOp2->getLHS());
                                    
                                    if(const IntegerLiteral *flagsLit1=dyn_cast<IntegerLiteral>(binOp3->getLHS()))
                                    {
                                        int flags = flagsLit1->getValue().getSExtValue();
                                        if(flags)
                                        {
                                            llvm::outs() << "Flags: " << flags << "\n";
                                            methodInfo.fgs[n] = flags;
                                            n++;
                                        }
                                    }
                                    if(const IntegerLiteral *flagsLit2=dyn_cast<IntegerLiteral>(binOp3->getRHS()))
                                    {
                                        int flags = flagsLit2->getValue().getSExtValue();
                                        if(flags)
                                        {
                                            llvm::outs() << "Flags: " << flags << "\n";
                                            methodInfo.fgs[n] = flags;
                                            n++;
                                        }                           
                                    }
                                }
                                if(const IntegerLiteral *flagsLit2=dyn_cast<IntegerLiteral>(binOp2->getRHS()))
                                {
                                    int flags = flagsLit2->getValue().getSExtValue();
                                    if(flags)
                                    {
                                        llvm::outs() << "Flags: " << flags << "\n";
                                        methodInfo.fgs[n] = flags;
                                        n++;
                                    }
                            
                                }
                            }
                            if(const IntegerLiteral *flagsLit2=dyn_cast<IntegerLiteral>(binOp->getRHS()))
                            {
                                int flags = flagsLit2->getValue().getSExtValue();
                                if(flags)
                                {
                                    llvm::outs() << "Flags: " << flags << "\n";
                                    methodInfo.fgs[n] = flags;
                                    n++;
                                }                           
                            }
                        }

                        if(const IntegerLiteral *flagsLit=dyn_cast<IntegerLiteral>(initElem)){
                            int flags = flagsLit->getValue().getSExtValue();
                            if(flags)
                            {
                                llvm::outs() << "Flags: " << flags << "\n";
                                methodInfo.flags = flags;
                            }                          
                        }
                        
                        
                        
                        methodInfos.push_back(methodInfo);// 保存该方法信息
                    }
                }
                
            }
        }
    }




 //以下对类进行分析

    //此时首先要确定类的PyTypeObject类型
    std::string typeStr=varDecl->getType().getAsString();
    if(typeStr=="PyTypeObject")
    {
        const Expr * initExpr0=varDecl->getInit();
        if(!initExpr0) return;
        
        
            if(const InitListExpr* initExpr=dyn_cast<InitListExpr>(initExpr0))
        {
             for(const auto & elem :initExpr->children())
            {
                 if(const Expr* subExpr=dyn_cast<Expr>(elem))
                 {
                     if(findDeclRefExpr(subExpr))
                     {
                        if(const DeclRefExpr* declrefexpr=dyn_cast<DeclRefExpr>(findDeclRefExpr(subExpr)))
                         {
                            llvm::outs()<<declrefexpr->getNameInfo().getAsString()<<"\n";

                        }

                }
                

                 }
                
             }
           

        }
           

        
        
        

    }



}

namespace clang::ento
{
// 动态注册 Checker
void registerPythonExtensionsChecker(CheckerManager &Mgr) {
    Mgr.registerChecker<PythonExtensionsChecker>();
}

// 注册时检查是否需要注册该 Checker
bool shouldRegisterPythonExtensionsChecker(const CheckerManager &Mgr) {
    return true;  // 在此可以根据需要设置条件
}
}
