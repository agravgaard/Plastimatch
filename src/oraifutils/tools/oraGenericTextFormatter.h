

#ifndef ORAGENERICTEXTFORMATTER_H_
#define ORAGENERICTEXTFORMATTER_H_

#include <map>
#include <stdlib.h>
#include <stdio.h>

#include "oraStringTools.h"


namespace ora
{


class VariableMetaData; // see below
class Operand;
class LogicalTreeTerm;
class DependencyTermPart;
class DependencyTerm;
class Modifier;

/**
 * Implements a generic text formatter which can handle variable text fragments
 * and further dependencies. This class is mainly meant for simple realization
 * of configurable / generic display text formatting.
 *
 * Typically a variable in text is wrapped within the following pattern "${}".
 * For example the variable "var1" might be found as "${var1}" within a text
 * that should get formatted.
 *
 * Formatting replaces the variable patterns in the text by their current
 * values.
 *
 * Additionally, formatting modifiers can be specified for each variable. The
 * following modifiers are available: <br>
 * ${/trim} ... trim string <br>
 * ${/ucase} ... string to upper case <br>
 * ${/lcase} ... string to lower case <br>
 * ${/iformat:format-string} ... integer-formatting <br>
 * ${/sformat:format-string} ... string-formatting <br>
 * ${/fformat:format-string} ... floating-point-formatting <br>
 * If a modifier should show an effect, it must be placed in front of the
 * according variable (without any spaces or other characters between), e.g.
 * "${/ucase}${VAR1}" would format the content of VAR1 in upper case letters.
 * More than one modifiers can be applied to a variable (concatenated from left
 * to right), e.g. "${/trim}${/ucase}${VAR1}" would first trim VAR1 and then
 * convert it to upper case letters. The format modifiers are type-specific (the
 * internal string-variable is casted to the specified type: iformat - integer,
 * uformat - unsigned integer, fformat - floating point number). Moreover, a
 * format-string must be supplied which refers to the typical C-format in
 * sprintf. For example "${/fformat:%3.5f}${VAR1}" would cast VAR1 internally
 * into a floating point number and then format it with at least 3 leading
 * numbers and a precision of 5 decimals.
 *
 * Moreover dependency patterns can be defined which are wrapped within the
 * following pattern "${d:} ... ${/d:}". For example the pattern
 * "${d:DATE} on ${DATE} ${/d:DATE}" says that the text " on ${DATE} " should
 * be discarded as a whole if the "DATE" variable is not specified (to avoid
 * idiotic text fragments). The closing term "${/d:DATE}" can also be written
 * more simple "${/d:}" (ATTENTION: there is no check on correspondence of
 * the label - each opening term must have a closing term!). Nesting these terms
 * is possible - but be careful! Moreover these dependency terms can have more
 * than one dependent variables which are logically connected (AND or OR). For
 * example the term "${d:DATE & TIME} on ${DATE} ${TIME} ${/d:DATE & TIME}" is
 * possible or this term "${d:DATE | TIME} on ${DATE} ${TIME} ${/d:DATE|TIME}".
 * NOTE: ANDs ("&") are generally stronger than ORs ("|"). Furthermore both are
 * binary operators and, therefore, need 2 operands. No empty spaces are
 * generally allowed between the operators and their operands. Be careful,
 * outer dependency terms can eliminate inner nested dependency terms by
 * definition. The operands can be more sophisticated (not only asking whether
 * a variable is defined or not), the supported operand-types are: <br>
 * DEFINED: by simply writing the variable name ("VAR") <br>
 * NOT DEFINED: by adding a "!"-character before the variable ("! VAR") <br>
 * EQUAL TO STRING (case-sensitive): by separating the variable and a string by
 * "==" ("VAR == STRING") <br>
 * EQUAL TO STRING (case-insensitive): by separating the variable and a
 * string by "="  ("VAR = STRING") <br>
 * LESS THAN STRING: by separating the variable and a string by "<"
 * ("VAR < STRING") <br>
 * GREATER THAN STRING: by separating the variable and a string by ">"
 * ("VAR > STRING") <br>
 * EQUAL TO INTEGER: by separating the variable and an integer-string by "==i"
 * ("VAR ==i INT") <br>
 * LESS THAN INTEGER: by separating the variable and an integer-string by "<i"
 * ("VAR <i INT") <br>
 * GREATER THAN STRING: by separating the variable and an integer-string by ">i"
 * ("VAR >i INT") <br>
 * EQUAL TO FLOAT: by separating the variable and a float-string by "==f"
 * ("VAR ==f FLOAT") <br>
 * LESS THAN FLOAT: by separating the variable and a float-string by "<f"
 * ("VAR <f FLOAT") <br>
 * GREATER THAN FLOAT: by separating the variable and a float-string by ">f"
 * ("VAR >f FLOAT") <br>
 * The separation needs not to include additional spaces, e.g. "VAR>fFLOAT" is
 * also valid. For example the following dependency term
 * "${d:VAR1 == test this | VAR1 == test that}COOL${/d:} "
 * would only result in "COOL"-output if VAR1 equaled "test this" or
 * "test that" (NOTE: the string argument is always implicitly trimmed!).
 *
 * NOTE: the performance of this class is relatively pure as, for example,
 * the logical term tree and the dependency terms are re-created each time
 * when FormatText() is called. That behaviour should be changed in future!
 *
 * @author phil 
 * @version 1.2
 */
class GenericTextFormatter
{
public:
  /** Default constructor **/
  GenericTextFormatter();
  /** Destructor **/
  virtual ~GenericTextFormatter();

  /** Clear all current variables information. **/
  virtual void ClearVariables();

  /**
   * Add a new supported variable.
   * @param variable the name of the variable (case-sensitive!)
   * @param initialValue optional initial value for the variable
   * @param description optional string expression describing the variable
   * @param emptyString optional string expression that defines the variable-
   * specific string that should be inserted into text when this variable is
   * not defined
   **/
  virtual void AddVariable(std::string variable, std::string initialValue = "",
      std::string description = "", std::string emptyString = "");

  /**
   * Update one of the current variables with a new value.
   * @param variable the name of the variable (case-sensitive!)
   * @param value the new value for the variable
   * @return TRUE if successful, FALSE otherwise (e.g. variable not found)
   */
  virtual bool UpdateVariable(std::string variable, std::string value);

  /**
   * @return a map containing the names of current supported variables (key)
   * and the description strings (value)
   */
  virtual std::map<std::string, std::string> GetVariablesAndDescriptions();

  /**
   * @return a map containing the names of current supported variables (key)
   * and the value strings (value)
   */
  virtual std::map<std::string, std::string> GetVariablesAndValues();

  /**
   * Format the specified text w.r.t. the internal configured variables.
   * @param text the text containing variable placeholders and other dependency
   * tags; this variable is directly modified by this method!
   * @return TRUE if the text could be formatted without any problems; FALSE
   * otherwise
   */
  virtual bool FormatText(std::string &text);

  /**
   * Format the specified text w.r.t. the internal configured variables.
   * @param text the text containing variable placeholders and other dependency
   * tags; this variable is NOT directly modified by this method - the result
   * is returned
   * @return result of the the text formatting operations (empty string if any
   * error occured)
   */
  virtual std::string FormatTextF(std::string &text);

  /**
   * @return information regarding the last ERROR which occurred during
   * formatting or syntax-checking
   */
  virtual std::string GetLastErrorMessage()
  {
    return m_LastErrorMessage;
  }

protected:
  /** map containing the variable identifiers and current values **/
  std::map<std::string, VariableMetaData *> m_Variables;
  /** last error message **/
  std::string m_LastErrorMessage;
  /** temporary logical term buffer **/
  LogicalTreeTerm *m_LogicalTreeTerm;
  /** temporary dependency terms (root term) **/
  DependencyTerm *m_DependencyTerms;

  /**
   * Do a syntax check on the specified text and do pre-processing for
   * formatting. WARNING: this method may not be perfect in detecting all
   * possible syntax errors!
   * @param text the text to be checked
   * @return TRUE if the syntax is OK
   */
  virtual bool CheckText(std::string &text);

  /**
   * Build a typical logical tree term from the specified string expression.
   * @param exp the string expression (see class-description for more
   * information on that)
   * @return TRUE if successful, FALSE otherwise (LastErrorMessage is written)
   */
  virtual bool BuildLogicalTreeTermFromExpression(std::string exp);

  /**
   * Format the specified text w.r.t. the internal configured variables.
   * NOTE: this method makes only sense when CheckText() returned TRUE on the
   * specified text!
   * @param text the text containing variable placeholders and other dependency
   * tags; this variable is directly modified by this method!
   * @return TRUE if the text could be formatted without any problems; FALSE
   * otherwise
   */
  virtual bool FormatTextInternally(std::string &text);

  /**
   * Evaluates the specified dependency term and its sub terms (recursively).
   * Terms which are currently not fulfilled (according to the logical tree
   * term) are removed (deleted) from the term 'tree' and deleted from text.
   * The positions of the other terms are adjusted! Fulfilled terms are
   * cleaned in text.
   * @param term - normally the root dependency term (without opener and closer)
   * from a user's perspective
   * @param text reference to the text string which is - probably - directly
   * modified
   * @return FALSE if the term must be deleted, TRUE if it should remain
   */
  virtual bool EvaluateAndRemoveDependencyTerms(DependencyTerm *term,
      std::string &text);

  /** Clean up internals. **/
  virtual void CleanUp();

};


/**
 * Help structure holding meta data for a variable.
 *
 * @author phil 
 * @version 1.0
 */
class VariableMetaData
{
public:
  /** Variable description **/
  std::string Description;
  /** Variable value **/
  std::string Value;
  /** Variable name **/
  std::string Variable;
  /** Empty string **/
  std::string EmptyString;

  /** Default constructor **/
  VariableMetaData()
  {
    Description = "";
    Value = "";
    Variable = "";
    EmptyString = "";
  }
  /** Destructor **/
  virtual ~VariableMetaData() { }

  /** @return the characteristic resulting variable string **/
  virtual std::string GetVariableString()
  {
    return "${" + Variable + "}";
  }

  /** @return whether this variable is defined (has a value) **/
  virtual bool IsDefined()
  {
    return (Variable.length() > 0 && Value.length() > 0);
  }
};


/**
 * Help structure for defining a specified operand for a logical term. An
 * operand, here, refers to an 'atomar' expression including:
 * DEFINED <br>
 * NOT DEFINED <br>
 * EQUAL TO (case-sensitive) string <br>
 * EQUAL TO (case-insensitive) string <br>
 * LESS THAN string <br>
 * GREATER THAN string <br>
 * EQUAL TO integer number <br>
 * LESS THAN integer number <br>
 * GREATER THAN integer number <br>
 * EQUAL TO floating point number <br>
 * LESS THAN floating point number <br>
 * GREATER THAN floating point number <br>
 *
 * @author phil 
 * @version 1.0
 */
class Operand
{
public:
  /** DEFINED ("VAR") **/
  static const int OP_DEFINED = 0;
  /** NOT DEFINED ("!VAR") **/
  static const int OP_NOT_DEFINED = 1;
  /** EQUAL TO STRING (case-sensitive) ("VAR==STRING") **/
  static const int OP_EQUAL_TO_STRING_CS = 2;
  /** EQUAL TO STRING (case-insensitive) ("VAR=STRING") **/
  static const int OP_EQUAL_TO_STRING_CIS = 3;
  /** LESS THAN STRING ("VAR<STRING") **/
  static const int OP_LESS_THAN_STRING = 4;
  /** GREATER THAN STRING ("VAR>STRING") **/
  static const int OP_GREATER_THAN_STRING = 5;
  /** EQUAL TO INTEGER ("VAR==iINT") **/
  static const int OP_EQUAL_TO_INT = 6;
  /** LESS THAN INTEGER ("VAR<iINT") **/
  static const int OP_LESS_THAN_INT = 7;
  /** GREATER THAN STRING ("VAR>iINT") **/
  static const int OP_GREATER_THAN_INT = 8;
  /** EQUAL TO FLOAT ("VAR==fFLOAT") **/
  static const int OP_EQUAL_TO_FLOAT = 9;
  /** LESS THAN FLOAT ("VAR<fFLOAT") **/
  static const int OP_LESS_THAN_FLOAT = 10;
  /** GREATER THAN FLOAT ("VAR>fFLOAT") **/
  static const int OP_GREATER_THAN_FLOAT = 11;

  /** operand type (one of the constants) **/
  int OperandType;
  /** optional arguments (e.g. for comparison) **/
  std::vector<std::string> Arguments;
  /** related variable **/
  VariableMetaData *Variable;

  /** Default constructor **/
  Operand()
  {
    OperandType = -1;
    Arguments.clear();
    Variable = NULL;
  }
  /** Destructor **/
  virtual ~Operand() { }

  /**
   * Fills this operand's attributes automatically from a specified operand
   * string.
   * @return TRUE if successful
   */
  virtual bool ApplyPropertiesFromOperandString(std::string operandString,
      std::map<std::string, VariableMetaData *> *variablesMap)
  {
    if (!variablesMap)
      return false;

    std::string s = TrimF(operandString);
    std::string varname = "";
    std::string::size_type p;

    OperandType = -1;
    Arguments.clear();
    Variable = NULL;

    if (s[0] == '!')
    {
      OperandType = OP_NOT_DEFINED;
      varname = s.substr(1); Trim(varname);
    }
    else if ((p = s.find("==f")) != std::string::npos)
    {
      OperandType = OP_EQUAL_TO_FLOAT;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 3)));
    }
    else if ((p = s.find("==i")) != std::string::npos)
    {
      OperandType = OP_EQUAL_TO_INT;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 3)));
    }
    else if ((p = s.find("==")) != std::string::npos)
    {
      OperandType = OP_EQUAL_TO_STRING_CS;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 2)));
    }
    else if ((p = s.find("=")) != std::string::npos)
    {
      OperandType = OP_EQUAL_TO_STRING_CIS;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 1)));
    }
    else if ((p = s.find("<f")) != std::string::npos)
    {
      OperandType = OP_LESS_THAN_FLOAT;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 2)));
    }
    else if ((p = s.find("<i")) != std::string::npos)
    {
      OperandType = OP_LESS_THAN_INT;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 2)));
    }
    else if ((p = s.find("<")) != std::string::npos)
    {
      OperandType = OP_LESS_THAN_STRING;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 1)));
    }
    else if ((p = s.find(">f")) != std::string::npos)
    {
      OperandType = OP_GREATER_THAN_FLOAT;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 2)));
    }
    else if ((p = s.find(">i")) != std::string::npos)
    {
      OperandType = OP_GREATER_THAN_INT;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 2)));
    }
    else if ((p = s.find(">")) != std::string::npos)
    {
      OperandType = OP_GREATER_THAN_STRING;
      varname = TrimF(s.substr(0, p));
      Arguments.push_back(TrimF(s.substr(p + 1)));
    }
    else // must be DEFINED
    {
      OperandType = OP_DEFINED;
      varname = s; Trim(varname);
    }

    // try to validate the variable
    std::map<std::string, VariableMetaData *>::iterator it =
      variablesMap->find(varname);
    if (it != variablesMap->end())
    {
      Variable = it->second;
      return true;
    }
    else
      return false;
  }

  /** Evaluate operand value (according to operand-type) and return it. **/
  virtual bool EvaluateOperand()
  {
    if (!Variable || OperandType < OP_DEFINED ||
        OperandType > OP_GREATER_THAN_FLOAT)
      return false;

    bool value = false;

    if (OperandType == OP_DEFINED)
      value = Variable->IsDefined();
    else if (OperandType == OP_NOT_DEFINED)
      value = !Variable->IsDefined();
    else if (OperandType == OP_EQUAL_TO_FLOAT && Arguments.size() > 0)
      value = (atof(Variable->Value.c_str()) ==
               atof(Arguments[0].c_str()));
    else if (OperandType == OP_EQUAL_TO_INT && Arguments.size() > 0)
      value = (atoi(Variable->Value.c_str()) ==
               atoi(Arguments[0].c_str()));
    else if (OperandType == OP_EQUAL_TO_STRING_CS && Arguments.size() > 0)
      value = (Variable->Value == Arguments[0]);
    else if (OperandType == OP_EQUAL_TO_STRING_CIS && Arguments.size() > 0)
      value = (ToLowerCaseF(Variable->Value) ==
               ToLowerCaseF(Arguments[0]));
    else if (OperandType == OP_LESS_THAN_FLOAT && Arguments.size() > 0)
      value = (atof(Variable->Value.c_str()) <
               atof(Arguments[0].c_str()));
    else if (OperandType == OP_LESS_THAN_INT && Arguments.size() > 0)
      value = (atoi(Variable->Value.c_str()) <
               atoi(Arguments[0].c_str()));
    else if (OperandType == OP_LESS_THAN_STRING && Arguments.size() > 0)
      value = (Variable->Value < Arguments[0]);
    else if (OperandType == OP_GREATER_THAN_FLOAT && Arguments.size() > 0)
      value = (atof(Variable->Value.c_str()) >
               atof(Arguments[0].c_str()));
    else if (OperandType == OP_GREATER_THAN_INT && Arguments.size() > 0)
      value = (atoi(Variable->Value.c_str()) >
               atoi(Arguments[0].c_str()));
    else if (OperandType == OP_GREATER_THAN_STRING && Arguments.size() > 0)
      value = (Variable->Value > Arguments[0]);

    return value;
  }

  /** @return the textual form of this operand **/
  virtual std::string ToString()
  {
    if (!Variable || OperandType < OP_DEFINED ||
        OperandType > OP_GREATER_THAN_FLOAT)
      return "";

    std::string s = "";

    if (OperandType == OP_DEFINED)
      s = Variable->GetVariableString();
    else if (OperandType == OP_NOT_DEFINED)
      s = "!" + Variable->GetVariableString();
    else if (OperandType == OP_EQUAL_TO_FLOAT && Arguments.size() > 0)
      s = Variable->GetVariableString() + " ==f " +
        StreamConvert(atof(Arguments[0].c_str()));
    else if (OperandType == OP_EQUAL_TO_INT && Arguments.size() > 0)
      s = Variable->GetVariableString() + " ==i " +
        StreamConvert(atoi(Arguments[0].c_str()));
    else if (OperandType == OP_EQUAL_TO_STRING_CS && Arguments.size() > 0)
      s = Variable->GetVariableString() + " == " + Arguments[0];
    else if (OperandType == OP_EQUAL_TO_STRING_CIS && Arguments.size() > 0)
      s = Variable->GetVariableString() + " = " + Arguments[0];
    else if (OperandType == OP_LESS_THAN_FLOAT && Arguments.size() > 0)
      s = Variable->GetVariableString() + " <f " +
        StreamConvert(atof(Arguments[0].c_str()));
    else if (OperandType == OP_LESS_THAN_INT && Arguments.size() > 0)
      s = Variable->GetVariableString() + " <i " +
        StreamConvert(atoi(Arguments[0].c_str()));
    else if (OperandType == OP_LESS_THAN_STRING && Arguments.size() > 0)
      s = Variable->GetVariableString() + " < " + Arguments[0];
    else if (OperandType == OP_GREATER_THAN_FLOAT && Arguments.size() > 0)
      s = Variable->GetVariableString() + " >f " +
        StreamConvert(atof(Arguments[0].c_str()));
    else if (OperandType == OP_GREATER_THAN_INT && Arguments.size() > 0)
      s = Variable->GetVariableString() + " >i " +
        StreamConvert(atoi(Arguments[0].c_str()));
    else if (OperandType == OP_GREATER_THAN_STRING && Arguments.size() > 0)
      s = Variable->GetVariableString() + " > " + Arguments[0];

    return s;
  }

};


/**
 * Help structure for storing and evaluating logical terms.
 *
 * @author phil 
 * @version 1.2
 */
class LogicalTreeTerm
{
public:
  /** operation type (TRUE = AND, FALSE = OR) **/
  bool ANDOperationType;
  /** operand type 1: atoms (variables which can be defined or undefined) **/
  std::vector<Operand *> Operands;
  /** operand type 2: nested sub-terms (again logical terms) **/
  std::vector<LogicalTreeTerm *> SubTerms;

  /** Default constructor **/
  LogicalTreeTerm()
  {
    ANDOperationType = true;
    Operands.clear();
    SubTerms.clear();
  }
  /** Destructor **/
  virtual ~LogicalTreeTerm()
  {
    Operands.clear();
    // delete children
    for (unsigned int i = 0; i < SubTerms.size(); i++)
      delete SubTerms[i];
    SubTerms.clear();
  }

  /**
   * Evaluate the tree recursively.
   * @return the evaluation result (boolean)
   */
  virtual bool Evaluate()
  {
    bool result;
    if (ANDOperationType) // AND
    {
      result = true;
      for (unsigned int i = 0; i < Operands.size(); i++)
        result = result && Operands[i]->EvaluateOperand();
      for (unsigned int i = 0; i < SubTerms.size(); i++)
        result = result && SubTerms[i]->Evaluate();
    }
    else // OR
    {
      result = false;
      for (unsigned int i = 0; i < Operands.size(); i++)
        result = result || Operands[i]->EvaluateOperand();
      for (unsigned int i = 0; i < SubTerms.size(); i++)
        result = result || SubTerms[i]->Evaluate();
    }
    return result;
  }

  /** @return the tree-hierarchy in textual form with actual boolean values **/
  virtual std::string ToString()
  {
    std::string txt = "";
    if (ANDOperationType) // AND
    {
      for (unsigned int i = 0; i < Operands.size(); i++)
      {
        if (txt.length() > 0)
          txt += " AND ";
        txt += Operands[i]->ToString() + " = " +
            StreamConvert(Operands[i]->EvaluateOperand());
      }
      for (unsigned int i = 0; i < SubTerms.size(); i++)
      {
        if (txt.length() > 0)
          txt += " AND ";
        txt += SubTerms[i]->ToString();
      }
      txt = "(" + txt + ")";
    }
    else // OR
    {
      for (unsigned int i = 0; i < Operands.size(); i++)
      {
        if (txt.length() > 0)
          txt += " OR ";
        txt += Operands[i]->ToString() + " = " +
            StreamConvert(Operands[i]->EvaluateOperand());
      }
      for (unsigned int i = 0; i < SubTerms.size(); i++)
      {
        if (txt.length() > 0)
          txt += " OR ";
        txt += SubTerms[i]->ToString();
      }
      txt = "(" + txt + ")";
    }
    return txt;
  }

};


/**
 * Help structure for storing information about a part of a dependency term.
 *
 * @author phil 
 * @version 1.0
 */
class DependencyTermPart
{
public:
  /** logical term referring to this dependency term part (opener) **/
  LogicalTreeTerm *Criterion;
  /** flag indicating whether it is an opener or closer **/
  bool Opener;
  /** start position of this dependency term part in text string **/
  std::string::size_type StartPosition;
  /** end position of this dependency term part in text string **/
  std::string::size_type EndPosition;

  /** Default constructor **/
  DependencyTermPart()
  {
    Criterion = NULL;
    Opener = false;
    StartPosition = -1;
    EndPosition = -1;
  }
  /** Destructor **/
  virtual ~DependencyTermPart()
  {
    // delete criterion!
    if (Criterion)
    {
      delete Criterion;
      Criterion = NULL;
    }
  }

};


/**
 * Help structure for storing information about a whole dependency term.
 *
 * @author phil 
 * @version 1.0
 */
class DependencyTerm
{
public:
  /** opener part **/
  DependencyTermPart *Opener;
  /** closer part **/
  DependencyTermPart *Closer;
  /** nested sub-dependency-terms **/
  std::vector<DependencyTerm *> SubTerms;
  /** depth level of term **/
  int Level;

  /** Default constructor **/
  DependencyTerm()
  {
    Opener = NULL;
    Closer = NULL;
    SubTerms.clear();
    Level = -1;
  }
  /** Destructor **/
  virtual ~DependencyTerm()
  {
    // delete members!
    if (Opener)
    {
      delete Opener;
      Opener = NULL;
    }
    if (Closer)
    {
      delete Closer;
      Closer = NULL;
    }
    for (unsigned int i = 0; i < SubTerms.size(); i++)
      delete SubTerms[i];
    SubTerms.clear();
  }

  /**
   * Translate this term and its sub terms by a specified number of characters.
   * @param diff amount of translation (negative - left translation, positive -
   * right translation)
   */
  virtual void TranslateTerm(int diff)
  {
    if (this->Opener)
    {
      this->Opener->StartPosition += diff;
      this->Opener->EndPosition += diff;
    }
    if (this->Closer)
    {
      this->Closer->StartPosition += diff;
      this->Closer->EndPosition += diff;
    }
    // recursively apply to sub terms, too:
    for (unsigned int i = 0; i < this->SubTerms.size(); i++)
      this->SubTerms[i]->TranslateTerm(diff);
  }

  /** @return the term-hierarchy in textual form **/
  virtual std::string ToString()
  {
    std::string txt = "";
    for (unsigned int i = 0; i < SubTerms.size(); i++)
    {
      if (txt.length() > 0)
        txt += " ; " + SubTerms[i]->ToString();
      else
        txt = SubTerms[i]->ToString();
    }
    if (Opener)
      txt = "S=" + StreamConvert(Opener->StartPosition) + " " + txt;
    if (Closer)
      txt = txt + " E=" + StreamConvert(Closer->EndPosition);
    txt = "(" + txt + ")";

    return txt;
  }

};


/**
 * Base class for variable format modifiers. For concrete modifiers sub classes
 * should be derived.
 *
 * @author phil 
 * @version 1.0
 */
class Modifier
{
public:
  /** some modifiers may require one or more arguments **/
  std::vector<std::string> Arguments;

  /** Default constructor **/
  Modifier()
  {
    Arguments.clear();
  }
  /** Destructor **/
  virtual ~Modifier() { }

  /**
   * This method is expected to set the modifier arguments from the pure
   * modifier command string. This base class implementation does not do
   * anything and does not declare this method as purely virtual because there
   * are modifiers which do not require any additional arguments.
   * @param modifierString the pure modifier command string (the string which is
   * enclosed by "${" and "}"
   * @return TRUE if the required arguments could successfully be extracted
   */
  virtual bool SetArgumentsFromPureModifierString(std::string modifierString)
  {
    Arguments.clear();
    return true;
  }

  /**
   * This method is expected to apply the configured modifier to the specified
   * string and return the result. NOTE: both input and output of the modifier
   * are always string-based. Internally the string can be casted to other types
   * (e.g. to numbers) in order to be able to apply the modifier correctly.
   * THIS METHOD MUST BE DEFINED IN CONCRETE SUBCLASSES!
   * @param variableString the string which contains the
   * @return the resultant string after application of the modifier
   */
  virtual std::string ApplyModifierToString(std::string variableString) = 0;

};

/**
 * Replaces the variable name (e.g. "ARG1") with its actual content (e.g.
 * "hello world!").
 *
 * @author phil 
 * @version 1.0
 */
class VariableContentModifier
  : public Modifier
{
public:
  /** pointer to an external map containing the variable meta data **/
  std::map<std::string, VariableMetaData *> *VariablesMap;

  /**
   * Replaces a variable name by its actual content. The variable is expected
   * to exclude the wrapping "${" and "}" strings! If the variable is not found,
   * and empty string will be returned.
   * @see Modifier#ApplyModifierToString()
   **/
  virtual std::string ApplyModifierToString(std::string variableString)
  {
    std::map<std::string, VariableMetaData *>::iterator it =
      VariablesMap->find(variableString);
    if (it != VariablesMap->end())
      return it->second->Value;
    else
      return "";
  }
};

/**
 * Trims the specified variable string.
 *
 * @author phil 
 * @version 1.0
 */
class TrimModifier
  : public Modifier
{
public:
  /**
   * Trims the specified variable string.
   * @see Modifier#ApplyModifierToString()
   **/
  virtual std::string ApplyModifierToString(std::string variableString)
  {
    return TrimF(variableString);
  }
};

/**
 * Turns the specified variable string to upper case.
 *
 * @author phil 
 * @version 1.0
 */
class UpperCaseModifier
  : public Modifier
{
public:
  /**
   * Turns the specified variable string to upper case.
   * @see Modifier#ApplyModifierToString()
   **/
  virtual std::string ApplyModifierToString(std::string variableString)
  {
    return ToUpperCaseF(variableString);
  }
};

/**
 * Turns the specified variable string to lower case.
 *
 * @author phil 
 * @version 1.0
 */
class LowerCaseModifier
  : public Modifier
{
public:
  /**
   * Turns the specified variable string to lower case.
   * @see Modifier#ApplyModifierToString()
   **/
  virtual std::string ApplyModifierToString(std::string variableString)
  {
    return ToLowerCaseF(variableString);
  }
};

/**
 * Cast the specified variable string to an integer and then apply a
 * specified formatting to that string.
 *
 * @author phil 
 * @version 1.0
 */
class IntegerFormatModifier
  : public Modifier
{
public:
  /**
   * @modifierString is expected to be of the form "/iformat:format-string"
   * where format-string is a typical C-format-string used for sprintf (e.g.
   * %5d). Example: "/iformat:%5d"
   * @see Modifier#SetArgumentsFromPureModifierString()
   */
  virtual bool SetArgumentsFromPureModifierString(std::string modifierString)
  {
    Arguments.clear();
    std::string::size_type p = modifierString.find(":");
    if (p != std::string::npos)
    {
      Arguments.push_back(TrimF(modifierString.substr(p + 1)));
      return true;
    }
    else
      return false;
  }

  /**
   * Cast the specified variable string to an integer and then apply a
   * specified formatting to that string. DO NOT FORGET TO USE THE
   * SetArgumentsFromPureModifierString() method before calling this method!
   * NOTE: the internal string size limit is 100 characters!
   * @see Modifier#ApplyModifierToString()
   **/
  virtual std::string ApplyModifierToString(std::string variableString)
  {
    if (Arguments.size() <= 0)
      return "";
    char buff[100];
    sprintf(buff, Arguments[0].c_str(), atoi(variableString.c_str()));
    return std::string(buff);
  }
};

/**
 * Cast the specified variable string to a floating point value and then apply a
 * specified formatting to that string.
 *
 * @author phil 
 * @version 1.0
 */
class FloatingPointFormatModifier
  : public Modifier
{
public:
  /**
   * @modifierString is expected to be of the form "/fformat:format-string"
   * where format-string is a typical C-format-string used for sprintf (e.g.
   * %5.4f). Example: "/fformat:%5.4f"
   * @see Modifier#SetArgumentsFromPureModifierString()
   */
  virtual bool SetArgumentsFromPureModifierString(std::string modifierString)
  {
    Arguments.clear();
    std::string::size_type p = modifierString.find(":");
    if (p != std::string::npos)
    {
      Arguments.push_back(TrimF(modifierString.substr(p + 1)));
      return true;
    }
    else
      return false;
  }

  /**
   * Cast the specified variable string to a floating point number and apply a
   * specified formatting to that string. DO NOT FORGET TO USE THE
   * SetArgumentsFromPureModifierString() method before calling this method!
   * NOTE: the internal string size limit is 100 characters!
   * @see Modifier#ApplyModifierToString()
   **/
  virtual std::string ApplyModifierToString(std::string variableString)
  {
    if (Arguments.size() <= 0)
      return "";
    char buff[100];
    sprintf(buff, Arguments[0].c_str(), atof(variableString.c_str()));
    return std::string(buff);
  }
};

/**
 * Apply a specified formatting to the specified variable string.
 *
 * @author phil 
 * @version 1.0
 */
class StringFormatModifier
  : public Modifier
{
public:
  /**
   * @modifierString is expected to be of the form "/sformat:format-string"
   * where format-string is a typical C-format-string used for sprintf (e.g.
   * %10s). Example: "/sformat:%10s"
   * @see Modifier#SetArgumentsFromPureModifierString()
   */
  virtual bool SetArgumentsFromPureModifierString(std::string modifierString)
  {
    Arguments.clear();
    std::string::size_type p = modifierString.find(":");
    if (p != std::string::npos)
    {
      Arguments.push_back(TrimF(modifierString.substr(p + 1)));
      return true;
    }
    else
      return false;
  }

  /**
   * Apply a specified formatting to the string. DO NOT FORGET TO USE THE
   * SetArgumentsFromPureModifierString() method before calling this method!
   * NOTE: the internal string size limit is 1000 characters!
   * @see Modifier#ApplyModifierToString()
   **/
  virtual std::string ApplyModifierToString(std::string variableString)
  {
    if (Arguments.size() <= 0)
      return "";
    char buff[1000];
    sprintf(buff, Arguments[0].c_str(), variableString.c_str());
    return std::string(buff);
  }
};


}


#endif /* ORAGENERICTEXTFORMATTER_H_ */
