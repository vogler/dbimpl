#include "Plan.hpp"
#include <fstream>
#include <algorithm>

namespace plan {

static std::string& toLower(std::string& str) {
   std::transform(str.begin(), str.end(), str.begin(), ::tolower);
   return str;
}

static bool isInt(const std::string& str) {
   return str.find_first_not_of("0123456789") == std::string::npos;
}

static std::unique_ptr<Operator> buildOperator(std::string& op) {
   toLower(op);
   if (op == "merge") {
      return std::move(std::unique_ptr<Operator>(new MergeJoin()));
   } else if (op == "sort") {
      return std::move(std::unique_ptr<Operator>(new Sort()));
   } else if (op == "select") {
      return std::move(std::unique_ptr<Operator>(new Select()));
   } else if (op == "project") {
      return std::move(std::unique_ptr<Operator>(new Project()));
   } else if (op == "scan") {
      return std::move(std::unique_ptr<Operator>(new TableScan()));
   } 
   throw;
}

static void readIntList(std::vector<unsigned>& ints, std::ifstream& i) {
   std::string tok;
   while (i.good()) {
      i >> tok;
      if (isInt(tok)) {
         ints.push_back(atoi(tok.c_str()));
      } else {
         for (unsigned j=tok.size(); j>0; --j)
            i.putback(tok[j-1]);
         break;
      }
   }
}

static Cmp readComparator(std::ifstream& i) {
   std::string tok;
   i >> tok;
   tok = toLower(tok);
   if (tok.compare("eq")==0)
      return Cmp::EQ;
   throw;
}

static Sort::Order readSortOrder(std::ifstream& i) {
   std::string tok;
   i >> tok;
   tok = toLower(tok);
   if (tok.compare("asc")==0)
      return Sort::Order::Asc;
   if (tok.compare("desc")==0)
      return Sort::Order::Desc;
   throw;
}

static void parse(Operator& op, std::ifstream& i) {
   switch (op.getOperatorType()) {
      case OperatorType::TableScan: {
         TableScan& t = static_cast<TableScan&>(op);
         i >> t.name;
         break;
      }
      case OperatorType::MergeJoin: {
         MergeJoin& m = static_cast<MergeJoin&>(op);
         readIntList(m.attributeIdsLeft, i);
         m.cmp = readComparator(i);
         readIntList(m.attributeIdsRight, i);
         std::string opCode;
         i >> opCode;
         std::unique_ptr<Operator> l(std::move(buildOperator(opCode)));
         parse(*l, i);
         m.setLeft(l);
         i >> opCode;
         std::unique_ptr<Operator> r(std::move(buildOperator(opCode)));
         parse(*r, i);
         m.setRight(r);
         break;
      }
      case OperatorType::Select: {
         Select& s = static_cast<Select&>(op);
         readIntList(s.attributeIds, i);
         s.cmp = readComparator(i);
         std::string constant;
         for (unsigned j=0, limit=s.attributeIds.size(); j<limit; ++j) {
            i >> constant;
            s.constants.push_back(constant);
         }
         std::string opCode;
         i >> opCode;
         std::unique_ptr<Operator> c(std::move(buildOperator(opCode)));
         parse(*c, i);
         s.setChild(c);
         break;
      }
      case OperatorType::Project: {
         Project& p = static_cast<Project&>(op);
         readIntList(p.attributeIds, i);
         std::string opCode;
         i >> opCode;
         std::unique_ptr<Operator> c(std::move(buildOperator(opCode)));
         parse(*c, i);
         p.setChild(c);
         break;
      }
      case OperatorType::Sort: {
         Sort& s = static_cast<Sort&>(op);
         readIntList(s.attributeIds, i);
         s.order = readSortOrder(i);
         std::string opCode;
         i >> opCode;
         std::unique_ptr<Operator> c(std::move(buildOperator(opCode)));
         parse(*c, i);
         s.setChild(c);
         break;
      }
      default: throw;
   }
}

bool Plan::fromFile(const char* fileName) {
   std::ifstream file(fileName);
   if (!file.is_open())
      return false;
   std::string token;
   file >> token;
   root = std::move(buildOperator(token));
   parse(*root, file);
   return true;
}

void Plan::print(std::ostream& out) const {
   root->print(out);
}
void Select::print(std::ostream& out, unsigned indent) const {
   out << std::string(indent, ' ') << "select";
   for (auto a : attributeIds)
      out << ' ' << a;
   switch (cmp) {
      case Cmp::EQ:
         out << ' ' << "eq";
         break;
      default:
         throw;
   }
   for (auto& c : constants) {
      out << ' ' << c;
   }
   out << std::endl;
   getChild().print(out, indent+1);
}
void Project::print(std::ostream& out, unsigned indent) const {
   out << std::string(indent, ' ') << "project";
   for (auto a : attributeIds)
      out << ' ' << a;
   out << std::endl;
   getChild().print(out, indent+1);
}
void TableScan::print(std::ostream& out, unsigned indent) const {
   out << std::string(indent, ' ') << "scan " << name << std::endl;
}
void MergeJoin::print(std::ostream& out, unsigned indent) const {
   out << std::string(indent, ' ') << "merge";
   for (auto a : attributeIdsLeft)
      out << ' ' << a;
   switch (cmp) {
      case Cmp::EQ:
         out << ' ' << "eq";
         break;
      default:
         throw;
   }
   for (auto a : attributeIdsRight)
      out << ' ' << a;
   out << std::endl;
   getLeft().print(out, indent+1);
   getRight().print(out, indent+1);
}
void Sort::print(std::ostream& out, unsigned indent) const {
   out << std::string(indent, ' ') << "sort";
   for (auto a : attributeIds)
      out << ' ' << a;
   switch (order) {
      case Sort::Order::Asc:
         out << ' ' << "asc";
         break;
      case Sort::Order::Desc:
         out << ' ' << "desc";
         break;
      default:
         throw;
   }
   out << std::endl;
   getChild().print(out, indent+1);
}

}