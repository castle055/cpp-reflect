
// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <clang/AST/AST.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/DeclarationName.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/PartialDiagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/Specifiers.h>
#include <clang/Sema/DeclSpec.h>
#include <clang/Serialization/ASTReader.h>
#include <cstdint>
#include <llvm/ADT/APInt.h>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/Decl.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Type.h"
#include "clang/Basic/DiagnosticSema.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/ParsedAttrInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;


//-----------------------------------------------------------------------------
// ASTConsumer
//-----------------------------------------------------------------------------
class ReflectorASTConsumer: public clang::ASTConsumer {
public:
  explicit ReflectorASTConsumer(CompilerInstance* CI, ASTContext* Ctx)
      : Compiler(CI),
        Context(Ctx) {

    if (Context->getCurrentNamedModule() && checkModuleUsable(Context->getCurrentNamedModule())) {
      // llvm::outs() << "[INFO] Adding static type information to module." << "\n";
    }
  }

  std::optional<std::reference_wrapper<IdentifierInfo>> pack_id {std::nullopt};
  std::optional<std::reference_wrapper<IdentifierInfo>> int_pack_id {std::nullopt};
  std::optional<TemplateName> PackTemplate {std::nullopt};
  std::optional<TemplateName> IntPackTemplate {std::nullopt};

  void add_types_decl(
    CXXRecordDecl* record, const std::deque<QualType>& field_types, const std::string& identifier
  ) {
    // Create the using field_types = pack<int, long, double> statement
    IdentifierInfo& FieldTypesID = Context->Idents.get(identifier);

    if (not pack_id.has_value()) {
      IdentifierInfo& PackID = Context->Idents.get("refl_pack");

      PackTemplate.emplace(dyn_cast<
        TemplateDecl>(Compiler->getSema().getAsTemplateNameDecl(Compiler->getSema().LookupSingleName(
        Compiler->getSema().getCurScope(), {&PackID}, record->getBeginLoc(), Sema::LookupOrdinaryName
      ))));
    }

    // Create Template Specialization for 'pack<int, long, double>'
    TemplateArgumentListInfo TemplateArgs{record->getBeginLoc(), record->getEndLoc()};
    for (const auto& type: field_types) {
      auto arg =
        TemplateArgumentLoc(TemplateArgument(type), Context->getTrivialTypeSourceInfo(type));
      TemplateArgs.addArgument(arg);
    }

    QualType PackSpecialization =
      Compiler->getSema().CheckTemplateIdType(PackTemplate.value(), record->getBeginLoc(), TemplateArgs);

    // Create TypeAliasDecl for 'using field_types = refl_pack<...>'
    TypeAliasDecl* FieldTypesAlias = TypeAliasDecl::Create(
      *Context,
      record,
      record->getBeginLoc(),
      record->getBeginLoc(),
      &FieldTypesID,
      Context->getTrivialTypeSourceInfo(PackSpecialization)
    );


    FieldTypesAlias->setAccess(AccessSpecifier::AS_public);

    // Add field_types to the struct
    record->addDecl(FieldTypesAlias);
  }

  TemplateName find_template(const std::string& identifier) {
    IdentifierInfo& PackID = Context->Idents.get(identifier);
    TemplateName    PackTemplate{dyn_cast<TemplateDecl>(
      Compiler->getSema().getAsTemplateNameDecl(Compiler->getSema().LookupSingleName(
        Compiler->getSema().getCurScope(), {&PackID}, SourceLocation(), Sema::LookupOrdinaryName
      ))
    )};
    return PackTemplate;
  }

  QualType specialize_template(const TemplateName& template_, TemplateArgumentListInfo& args) {
    return Compiler->getSema().CheckTemplateIdType(template_, SourceLocation{}, args);
  }

  void add_integer_list(
    CXXRecordDecl* record, const std::deque<uint64_t>& items, const std::string& identifier
  ) {
    // Create Template Specialization for 'pack<int, long, double>'
    TemplateArgumentListInfo TemplateArgs{record->getBeginLoc(), record->getEndLoc()};
    QualType                 type = Context->UnsignedLongTy;
    for (const auto& item: items) {
      auto  value     = llvm::APInt(64, std::to_string(item), 10);
      auto* item_expr = IntegerLiteral::Create(*Context, value, type, record->getBeginLoc());
      TemplateArgs.addArgument(TemplateArgumentLoc(
        TemplateArgument(*Context, type, APValue(llvm::APSInt(value))), item_expr
      ));
    }

    if (not int_pack_id.has_value()) {
      IdentifierInfo& PackID = Context->Idents.get("refl_int_pack");

      IntPackTemplate.emplace(dyn_cast<
        TemplateDecl>(Compiler->getSema().getAsTemplateNameDecl(Compiler->getSema().LookupSingleName(
        Compiler->getSema().getCurScope(), {&PackID}, record->getBeginLoc(), Sema::LookupOrdinaryName
      ))));
    }

    QualType PackSpecialization = specialize_template(IntPackTemplate.value(), TemplateArgs);

    // Create TypeAliasDecl for 'using field_types = pack<int, long, double>'
    IdentifierInfo& FieldTypesID    = Context->Idents.get(identifier);
    TypeAliasDecl*  FieldTypesAlias = TypeAliasDecl::Create(
      *Context,
      record,
      record->getBeginLoc(),
      record->getLocation(),
      &FieldTypesID,
      Context->getTrivialTypeSourceInfo(PackSpecialization)
    );

    FieldTypesAlias->setAccess(AccessSpecifier::AS_public);

    // Add field_types to the struct
    record->addDecl(FieldTypesAlias);
  }

  void add_names_decl(
    CXXRecordDecl* record, const std::deque<std::string>& names, const std::string& identifier
  ) {
    IdentifierInfo& FieldNamesID          = Context->Idents.get(identifier);
    QualType        ConstCharPtrArrayType = Context->getConstantArrayType(
      Context->getPointerType(Context->CharTy.withConst()),
      llvm::APInt(32, names.size()),
      nullptr,
      ArraySizeModifier::Normal,
      0
    );
    TypeSourceInfo* TSI = Context->getTrivialTypeSourceInfo(ConstCharPtrArrayType);

    VarDecl* FieldNamesVar = VarDecl::Create(
      *Context,
      record,
      record->getBeginLoc(),
      record->getBeginLoc(),
      &FieldNamesID,
      ConstCharPtrArrayType,
      nullptr,
      SC_Static
    );
    FieldNamesVar->setInitStyle(VarDecl::CInit);
    FieldNamesVar->setTypeSourceInfo(TSI);

    // Create the array initializer
    SmallVector<Expr*, 1> init_exprs{};
    for (const auto& name: names) {
      init_exprs.push_back(StringLiteral::Create(
        *Context,
        name,
        StringLiteralKind::Ordinary,
        false,
        Context->getStringLiteralArrayType(Context->CharTy.withConst(), name.size()),
        record->getBeginLoc()
      ));
    }


    auto* FieldNamesInitList =
      new (Context) InitListExpr(*Context, record->getBeginLoc(), init_exprs, record->getEndLoc());

    FieldNamesInitList->setType(ConstCharPtrArrayType);
    FieldNamesVar->setConstexpr(true);
    FieldNamesVar->setAccess(AccessSpecifier::AS_public);
    Compiler->getSema().AddInitializerToDecl(FieldNamesVar, FieldNamesInitList, false);

    // // Add field_names to the struct
    record->addDecl(FieldNamesVar);
  }

  void add_metadata_decl(
    CXXRecordDecl*                     record,
    const std::deque<std::list<Expr*>>& metadata_exprs,
    const std::string&                 identifier
  ) {
    IdentifierInfo& MetadataID = Context->Idents.get(identifier);
    IdentifierInfo& TupleID    = Context->Idents.get("refl_tuple");

    // Create Template Specialization for 'tuple<T...>'
    auto lookup_res = Compiler->getSema().LookupSingleName(
      Compiler->getSema().getCurScope(), {&TupleID}, record->getBeginLoc(), Sema::LookupOrdinaryName
    );
    if (nullptr == lookup_res) {
      return;
    }
    TemplateName TupleTemplate{
      dyn_cast<TemplateDecl>(Compiler->getSema().getAsTemplateNameDecl(lookup_res))
    };

    TemplateArgumentListInfo TemplateArgs{record->getBeginLoc(), record->getEndLoc()};
    std::vector<Expr*>       init_expr_lists{};
    for (const auto& metadata_expr_list: metadata_exprs) {
      for (const auto& expr: metadata_expr_list) {
        auto et = expr->getType();
        if (et->isArrayType()) {
          et = Context->getPointerType(et->getAsArrayTypeUnsafe()->getElementType());
        }
        auto arg = TemplateArgumentLoc(TemplateArgument(et), Context->getTrivialTypeSourceInfo(et));
        TemplateArgs.addArgument(arg);
        init_expr_lists.push_back(expr);
      }
    }

    Expr* initializer = new (Context)
      InitListExpr(*Context, record->getBeginLoc(), init_expr_lists, record->getEndLoc());

    QualType TupleSpecialization =
      Compiler->getSema()
        .CheckTemplateIdType(TupleTemplate, record->getBeginLoc(), TemplateArgs)
        .withConst();
    TypeSourceInfo* TSI = Context->getTrivialTypeSourceInfo(TupleSpecialization);

    //=====

    VarDecl* MetadataVar = VarDecl::Create(
      *Context,
      record,
      record->getBeginLoc(),
      record->getBeginLoc(),
      &MetadataID,
      TupleSpecialization,
      nullptr,
      SC_Static
    );
    MetadataVar->setInitStyle(VarDecl::CInit);
    MetadataVar->setTypeSourceInfo(TSI);
    MetadataVar->setImplicitlyInline();
    initializer->setType(TupleSpecialization);
    MetadataVar->setConstexpr(true);
    MetadataVar->setAccess(AccessSpecifier::AS_public);

    Compiler->getSema().AddInitializerToDecl(MetadataVar, initializer, false);

    // if (init_expr_lists.size() > 0) {
    //   auto* ewc = dyn_cast<ExprWithCleanups>(MetadataVar->getInit());
    //   ewc->getSourceRange();
    //   initializer->dumpColor();
    //   MetadataVar->getInit()->dumpColor();
    //   // MetadataVar->dumpColor();
    //   // for (auto lookup_constructor :
    //   // Compiler->getSema().LookupConstructors(TupleSpecialization->getAsCXXRecordDecl()))
    //   // lookup_constructor->dumpColor();
    // }
    MetadataVar->ensureEvaluatedStmt();
    record->addDecl(MetadataVar);
  }

  void add_type_info(CXXRecordDecl* record) {
    std::deque<std::string> field_names{};
    std::deque<uint64_t>    field_sizes{};
    std::deque<uint64_t>    field_offsets{};
    std::deque<uint64_t>    field_accesses{};
    std::deque<QualType>    field_types{};

    std::deque<std::string> method_names{};
    std::deque<uint64_t>    method_accesses{};
    std::deque<QualType>    method_types{};

    uint64_t                    last_metadata_offset = 0;
    std::deque<uint64_t>         field_metadata_offsets{};
    std::deque<uint64_t>         field_metadata_counts{};
    std::deque<std::list<Expr*>> field_metadata_exprs{};

    for (const auto& field: record->fields()) {
      if (field->isTemplated() || field->isTemplateDecl())
        continue;

      auto name   = field->getNameAsString();
      auto type   = field->getType();
      auto size   = Context->getTypeSize(type);
      auto offset = Context->getFieldOffset(field) >> 3;

      uint64_t access = 0;
      switch (field->getAccess()) {
        case AS_public:
          access = 3;
          break;
        case AS_protected:
          access = 2;
          break;
        case AS_private:
          access = 1;
          break;
        case AS_none:
          access = 0;
          break;
      }

      std::list<Expr*> metadata_exprs{};
      bool             ignored         = false;
      bool             has_annotations = false;
      for (const auto& attr: field->attrs()) {
        // llvm::outs() << "Field has attr: ";
        // attr->printPretty(llvm::outs(), Context->getPrintingPolicy());
        // llvm::outs() << "\n";
        if (isa<AnnotateAttr>(attr)) {
          has_annotations         = true;
          AnnotateAttr* AnnotAttr = dyn_cast<AnnotateAttr>(attr);
          // llvm::outs() << "Field has annotation: " << AnnotAttr->getAnnotation();

          if (AnnotAttr->getAnnotation() == "refl::ignore") {
            ignored = true;
          } else if (AnnotAttr->getAnnotation() == "meta") {
            for (auto& arg: AnnotAttr->args()) {
              metadata_exprs.push_back(arg);
              // arg = nullptr;
            }
          }
        }
      }
      if (has_annotations) {
        field->dropAttr<AnnotateAttr>();
      }

      if (ignored) {
        continue;
      }

      field_names.push_back(name);
      field_sizes.push_back(size);
      field_offsets.push_back(offset);
      field_types.push_back(type);
      field_accesses.push_back(access);
      field_metadata_offsets.push_back(last_metadata_offset);
      field_metadata_counts.push_back(metadata_exprs.size());
      last_metadata_offset += metadata_exprs.size();
      field_metadata_exprs.push_back(metadata_exprs);
    }

    for (const auto& method: record->methods()) {
      if (method->isTemplated() || method->isTemplateDecl())
        continue;

      auto name = method->getNameAsString();
      auto type = method->getType();

      uint64_t access = 0;
      switch (method->getAccess()) {
        case AS_public:
          access = 3;
          break;
        case AS_protected:
          access = 2;
          break;
        case AS_private:
          access = 1;
          break;
        case AS_none:
          access = 0;
          break;
      }

      method_names.push_back(name);
      method_types.push_back(type);
      method_accesses.push_back(access);
    }

    IdentifierInfo& type_info_id     = Context->Idents.get("__type_info__");
    CXXRecordDecl*  type_info_record = CXXRecordDecl::Create(
      *Context,
      CXXRecordDecl::TagKind::Class,
      record,
      record->getBeginLoc(),
      record->getLocation(),
      &type_info_id
    );

    type_info_record->startDefinition();

    //    if (!field_names.empty()) {
    add_names_decl(type_info_record, field_names, "field_names");
    add_types_decl(type_info_record, field_types, "field_types");
    add_integer_list(type_info_record, field_sizes, "field_sizes");
    add_integer_list(type_info_record, field_offsets, "field_offsets");
    add_integer_list(type_info_record, field_accesses, "field_access_specifiers");
    add_metadata_decl(type_info_record, field_metadata_exprs, "field_metadata");
    add_integer_list(type_info_record, field_metadata_offsets, "field_metadata_offsets");
    add_integer_list(type_info_record, field_metadata_counts, "field_metadata_counts");
    //    }

    //    if (!method_names.empty()) {
    add_names_decl(type_info_record, method_names, "method_names");
    add_types_decl(type_info_record, method_types, "method_types");
    add_integer_list(type_info_record, method_accesses, "method_access_specifiers");
    //    }

    type_info_record->setAccess(AccessSpecifier::AS_public);
    type_info_record->completeDefinition();
    // Compiler->getSema().CheckCompletedCXXClass(Compiler->getSema().getCurScope(), type_info_record);
    record->addDecl(type_info_record);
  }


  void HandleTagDeclDefinition(TagDecl* D) override {
    if (!module_usable && ((D->getOwningModule() == nullptr) ||
                           D->getOwningModule()->getPrimaryModuleInterfaceName() != "reflect")) {
      return;
    }

    if ((D->isStruct() || D->isClass()) && (D->getDefinition() == D) && !D->isEnum()
    /* &&
        !D->isInAnotherModuleUnit() &&
        (D->hasOwningModule() && !D->getOwningModule()->isGlobalModule())*/) {
      if (D->isTemplated() and not isa<ClassTemplateSpecializationDecl>(D)) {
        return;
      }
      auto* record = dyn_cast<CXXRecordDecl>(D->getDefinition());
      add_type_info(record);
      // if (record->getName() == "test_one_field_struct") {
        // record->dumpColor();
        //   for (auto d: record->decls()) {
        //     d->dumpColor();
        //   }
      // }
    }
  }

  bool HandleTopLevelDecl(DeclGroupRef D) override {
    // llvm::outs() << "HANDLING TOP LEVEL DECLARATION" << "\n";
    // IdentifierInfo& PackID = Context->Idents.get("pack");
    // auto*           lookup = Compiler->getSema().LookupSingleName(
    //   Compiler->getSema().getCurScope(), {&PackID}, SourceLocation(), Sema::LookupOrdinaryName
    // );
    // if (lookup != nullptr) {
    //   llvm::outs() << "[WARN] MODULE USABLE " << "\n";
    //   module_usable = true;
    //   return true;
    // }

    // return true;
    for (const auto& d: D) {
      if (d->getKind() == Decl::Import) {
        ImportDecl* id = dyn_cast<ImportDecl>(d);
        // llvm::outs() << "[ERROR] import "
        //              << id->getImportedModule()->getPrimaryModuleInterfaceName() << "\n";
        if (!module_usable && checkModuleUsable(id->getImportedModule())) {
          // llvm::outs() << "[WARN] MODULE USABLE " << "\n";
        }
      }

      // d->print(llvm::outs());
    }
    return true;
  }

private:
  bool checkModuleUsable(Module* m, unsigned int indent = 0) {
    // for (unsigned int i = 0; i < indent; ++i) {
    //   llvm::outs() << "  ";
    // }
    // llvm::outs() << m->getFullModuleName() << "\n";

    if (m->getPrimaryModuleInterfaceName() == "reflect") {
      refl_module   = m;
      module_usable = true;
      return true;
    }

    SmallVector<Module*> exported_modules{};
    m->getExportedModules(exported_modules);
    for (const auto& exported_module: exported_modules) {
      if (checkModuleUsable(exported_module, indent + 1)) {
        return true;
      }
    }

    return false;
  }

private:
  CompilerInstance* Compiler;
  ASTContext*       Context;
  bool              module_usable = false;
  Module*           refl_module   = nullptr;
};
