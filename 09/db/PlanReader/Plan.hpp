#ifndef H_Plan_HPP
#define H_Plan_HPP

#include <memory>
#include <vector>
#include <string>

namespace plan {

enum class OperatorType { Select, Project, Sort, MergeJoin, TableScan };
enum class Cmp { EQ /*equality*/ }; // Compare operation

struct Operator {
   virtual OperatorType getOperatorType() const = 0;
   virtual void print(std::ostream& out, unsigned indent=0) const = 0;
};

class BinaryOperator : public Operator {
   std::unique_ptr<Operator> left;
   std::unique_ptr<Operator> right;
public:
   const Operator& getLeft() const { return *left; };
   const Operator& getRight() const { return *right; };
   void setLeft(std::unique_ptr<Operator>& l) { left = std::move(l); }
   void setRight(std::unique_ptr<Operator>& r) { right = std::move(r); }
};

class UnaryOperator : public Operator {
   std::unique_ptr<Operator> child;
public:
   const Operator& getChild() const { return *child; };
   void setChild(std::unique_ptr<Operator>& c) { child = std::move(c); }
};

struct LeafOperator : public Operator {
};


struct Select : public UnaryOperator {
   std::vector<unsigned> attributeIds;
   Cmp cmp;
   std::vector<std::string> constants;
   OperatorType getOperatorType() const { return OperatorType::Select; }
   void print(std::ostream& out, unsigned indent=0) const;
};

struct Project : public UnaryOperator {
   std::vector<unsigned> attributeIds;
   OperatorType getOperatorType() const { return OperatorType::Project; }
   void print(std::ostream& out, unsigned indent=0) const;
};

struct Sort : public UnaryOperator {
   enum class Order { Asc, Desc };
   std::vector<unsigned> attributeIds;
   Order order;
   OperatorType getOperatorType() const { return OperatorType::Sort; }
   void print(std::ostream& out, unsigned indent=0) const;
};

struct TableScan : public LeafOperator {
   std::string name;
   OperatorType getOperatorType() const { return OperatorType::TableScan; }
   void print(std::ostream& out, unsigned indent=0) const;
};

struct MergeJoin : public BinaryOperator {
   std::vector<unsigned> attributeIdsLeft;
   Cmp cmp;
   std::vector<unsigned> attributeIdsRight;
   OperatorType getOperatorType() const { return OperatorType::MergeJoin; }
   void print(std::ostream& out, unsigned indent=0) const;
};

class Plan {
   std::unique_ptr<Operator> root;
public:
   bool fromFile(const char* fileName);
   const Operator& getRoot() const { return *root; };
   void print(std::ostream& out) const;
};
}
#endif