// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ASTConsumer.h"

//-----------------------------------------------------------------------------
// FrontendAction for Reflector Plugin
//-----------------------------------------------------------------------------
class ReflectorAction: public clang::PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI, StringRef InFile) override {
    // Module* m = CI.getPreprocessor().getHeaderSearchInfo().lookupModule("fabric.refl");
    // if (m && m->isAvailable() && !m->isUnimportable()) {
    //     llvm::outs() << "[ERROR] MODULE ACCESSIBLE " << m->getPrimaryModuleInterfaceName() <<
    //     "\n";
    // }
    return std::unique_ptr<clang::ASTConsumer>(
      std::make_unique<ReflectorASTConsumer>(&CI, &CI.getASTContext())
    );
  }


  bool ParseArgs(const CompilerInstance& CI, const std::vector<std::string>& args) override {
    return true;
  }

  ActionType getActionType() override {
    return ActionType::AddBeforeMainAction;
  }
};

//-----------------------------------------------------------------------------
// Registration
//-----------------------------------------------------------------------------
static FrontendPluginRegistry::Add<ReflectorAction>
  X("reflector", "Adds static type information to structs and classes.");

namespace {
  struct ExampleAttrInfo: public ParsedAttrInfo {
    ExampleAttrInfo() {
      NumArgs                       = 1;
      OptArgs                       = 0;
      static constexpr Spelling S[] = {
        {ParsedAttr::AS_GNU, "meta"},
        {ParsedAttr::AS_C23, "meta"},
        {ParsedAttr::AS_CXX11, "meta"},
        {ParsedAttr::AS_CXX11, "refl::meta"},
      };
      Spellings = S;
    }

    bool diagAppertainsToDecl(Sema& S, const ParsedAttr& Attr, const Decl* D) const override {
      // This attribute appertains to fields only.
      if (!isa<FieldDecl>(D)) {
        S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type_str) << Attr << "fields";
        return false;
      }

      return true;
    }

    AttrHandling handleDeclAttribute(Sema& S, Decl* D, const ParsedAttr& Attr) const override {
      Expr** expr = new (D->getASTContext()) Expr*();
      *expr       = nullptr;

      if (Attr.isArgExpr(0)) {
        Expr* expr_ = Attr.getArgAsExpr(0);
        // expr_->dumpColor();
        // if (isa<CXXFunctionalCastExpr>(expr_)) {
        //   // expr_->getType().dump();
        //   // CXXFunctionalCastExpr& e{*cast<CXXFunctionalCastExpr>(expr_)};
        //
        //   // *expr = CXXFunctionalCastExpr::CreateEmpty(D->getASTContext(), 0, false);
        //   // CXXFunctionalCastExpr& ee{*cast<CXXFunctionalCastExpr>(*expr)};
        //   // ee.setLParenLoc(e.getLParenLoc());
        //   // ee.setRParenLoc(e.getRParenLoc());
        //   // ee.setCastKind(e.getCastKind());
        //   // ee.setObjectKind(e.getObjectKind());
        //   // ee.setSubExpr(e.getSubExpr());
        //   // ee.setType(e.getType());
        //   // ee.setTypeInfoAsWritten(e.getTypeInfoAsWritten());
        //   // ee.setValueKind(e.getValueKind());
        //   (*expr) = expr_;
        // } else
         if (isa<StringLiteral>(expr_)) {
          // expr_->getType().dump();
          StringLiteral& e{*cast<StringLiteral>(expr_)};

          StringLiteral* sexpr = StringLiteral::Create(
            S.getASTContext(),
            e.getString(),
            StringLiteralKind::Ordinary,
            false,
            D->getASTContext().getStringLiteralArrayType(
              D->getASTContext().CharTy.withConst(), e.getByteLength()
            ),
            // D->getASTContext().getPointerType(D->getASTContext().CharTy.withConst()),
            e.getBeginLoc()
          );
          (*expr) = sexpr;
        } else if (isa<IntegerLiteral>(expr_)) {
          (*expr) = expr_;
        } else {
          // S.EvaluateConvertedConstantExpression();
          APValue v{};
          APValue pv{};
          S.EvaluateConvertedConstantExpression(expr_, expr_->getType(), v, Sema::CCEK_Noexcept, false, pv);

          auto* cexpr = ConstantExpr::Create(D->getASTContext(), expr_, v);
          // cexpr->setType(expr_->getType().withConst());
          (*expr) = cexpr; //S.ActOnConstantExpression(cexpr).get();
        }
      } else if (Attr.isArgIdent(0)) {
        // TODO
      }

      if (nullptr != *expr) {
        D->addAttr(AnnotateAttr::Create(S.Context, "meta", expr, 1, Attr.getLoc()));
        return AttributeApplied;
      }
      return AttributeNotApplied;
    }
  };
} // namespace

namespace {
  struct ReflIgnoreAttrInfo: public ParsedAttrInfo {
    ReflIgnoreAttrInfo() {
      NumArgs                       = 0;
      OptArgs                       = 0;
      static constexpr Spelling S[] = {
        {ParsedAttr::AS_GNU, "refl::ignore"},
        {ParsedAttr::AS_C23, "refl::ignore"},
        {ParsedAttr::AS_CXX11, "refl::ignore"},
        {ParsedAttr::AS_CXX11, "refl::ignore"},
      };
      Spellings = S;
    }

    bool diagAppertainsToDecl(Sema& S, const ParsedAttr& Attr, const Decl* D) const override {
      // This attribute appertains to fields only.
      if (!isa<FieldDecl>(D)) {
        S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type_str) << Attr << "fields";
        return false;
      }

      return true;
    }

    AttrHandling handleDeclAttribute(Sema& S, Decl* D, const ParsedAttr& Attr) const override {
      D->addAttr(AnnotateAttr::Create(S.Context, "refl::ignore", nullptr, 0, Attr.getRange()));
      return AttributeApplied;
    }
  };

  struct ReflAnnotationAttrInfo: public ParsedAttrInfo {
    ReflAnnotationAttrInfo() {
      NumArgs                       = 0;
      OptArgs                       = 15;
      static constexpr Spelling S[] = {
        {ParsedAttr::AS_CXX11, "refl::annotation"},
        {ParsedAttr::AS_GNU, "refl::annotation"},
        {ParsedAttr::AS_C23, "refl::annotation"},
      };
      Spellings = S;
    }

    bool diagAppertainsToDecl(Sema& S, const ParsedAttr& Attr, const Decl* D) const override {
      if (isa<CXXRecordDecl>(D)) {
        return true;
      }
      return false;
    }

    AttrHandling handleDeclAttribute(Sema& S, Decl* D, const ParsedAttr& Attr) const override {

      // add_static_string_field(S, cast<CXXRecordDecl>(D), "__name__", )
      // D->addAttr(AnnotateAttr::Create(S.Context, "ignore", nullptr, 0, Attr.getRange()));
      return AttributeApplied;
    }

  private:
    void add_static_string_field(
      Sema& S, CXXRecordDecl* record, const std::string& identifier, const std::string& string
    ) {
      auto*           Context          = &record->getASTContext();
      IdentifierInfo& FieldNameID      = Context->Idents.get(identifier);
      QualType        ConstCharPtrType = Context->getPointerType(Context->CharTy.withConst());
      TypeSourceInfo* TSI              = Context->getTrivialTypeSourceInfo(ConstCharPtrType);

      VarDecl* FieldVar = VarDecl::Create(
        *Context,
        record,
        SourceLocation(),
        SourceLocation(),
        &FieldNameID,
        ConstCharPtrType,
        nullptr,
        SC_Static
      );
      FieldVar->setInitStyle(VarDecl::CInit);
      FieldVar->setTypeSourceInfo(TSI);

      // Create the array initializer
      Expr* init_expr = StringLiteral::Create(
        *Context,
        string,
        StringLiteralKind::Ordinary,
        false,
        Context->getStringLiteralArrayType(Context->CharTy.withConst(), string.size()),
        SourceLocation()
      );

      init_expr->setType(ConstCharPtrType);
      FieldVar->setConstexpr(true);
      FieldVar->setAccess(AccessSpecifier::AS_public);
      S.AddInitializerToDecl(FieldVar, init_expr, false);

      // // Add field_names to the struct
      record->addDecl(FieldVar);
    }
  };
} // namespace

static ParsedAttrInfoRegistry::Add<ReflAnnotationAttrInfo> A("refl-annotations", "");
static ParsedAttrInfoRegistry::Add<ReflIgnoreAttrInfo>     I("refl-ignore", "");
static ParsedAttrInfoRegistry::Add<ExampleAttrInfo>        M("refl-metadata", "");
