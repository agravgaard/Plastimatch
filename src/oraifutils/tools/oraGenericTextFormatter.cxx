

#include "oraGenericTextFormatter.h"


#include <sstream>


namespace ora
{


GenericTextFormatter
::GenericTextFormatter()
{
  m_Variables.clear();
  m_LogicalTreeTerm = NULL;
  m_DependencyTerms = NULL;
}

GenericTextFormatter
::~GenericTextFormatter()
{
  ClearVariables();
  CleanUp();
}

void
GenericTextFormatter
::ClearVariables()
{
  std::map<std::string, VariableMetaData *>::iterator it;
  for (it = m_Variables.begin(); it != m_Variables.end(); ++it)
    delete it->second;
  m_Variables.clear();
}

void
GenericTextFormatter
::AddVariable(std::string variable, std::string initialValue,
    std::string description, std::string emptyString)
{
  std::map<std::string, VariableMetaData *>::iterator it = m_Variables.find(
      variable);
  if (it == m_Variables.end())
  {
    VariableMetaData *md = new VariableMetaData();
    md->Value = initialValue;
    md->Description = description;
    md->Variable = variable;
    md->EmptyString = emptyString;
    m_Variables.insert(std::pair<std::string, VariableMetaData *>(variable, md));
  }
  else // simply update if not new
    UpdateVariable(variable, initialValue);
}

bool
GenericTextFormatter
::UpdateVariable(std::string variable, std::string value)
{
  std::map<std::string, VariableMetaData *>::iterator it = m_Variables.find(
      variable);
  if (it != m_Variables.end() && it->second)
  {
    it->second->Value = value;
    return true;
  }
  else
    return false;
}

std::map<std::string, std::string>
GenericTextFormatter
::GetVariablesAndDescriptions()
{
  std::map<std::string, std::string> ret;
  std::map<std::string, VariableMetaData *>::iterator it;
  for (it = m_Variables.begin(); it != m_Variables.end(); ++it)
  {
    if (it->second)
      ret.insert(std::pair<std::string, std::string>(it->first,
          it->second->Description));
  }
  return ret;
}

std::map<std::string, std::string>
GenericTextFormatter
::GetVariablesAndValues()
{
  std::map<std::string, std::string> ret;
  std::map<std::string, VariableMetaData *>::iterator it;
  for (it = m_Variables.begin(); it != m_Variables.end(); ++it)
  {
    if (it->second)
      ret.insert(std::pair<std::string, std::string>(it->first,
          it->second->Value));
  }
  return ret;
}

bool
GenericTextFormatter
::EvaluateAndRemoveDependencyTerms(DependencyTerm *term, std::string &text)
{
  bool remain = true;

  if (!term)
    return remain; // not defined - so do not delete

  if (term->Opener && term->Opener->Criterion && term->Closer)
  {
    remain = term->Opener->Criterion->Evaluate();
    if (remain)
    {
      // clean text
      text.erase(term->Closer->StartPosition,
          term->Closer->EndPosition - term->Closer->StartPosition + 1);
      text.erase(term->Opener->StartPosition,
          term->Opener->EndPosition - term->Opener->StartPosition + 1);

      // adjust other terms
      // - sub terms between this term's bounds (subtract opener size only)
      int odiff = term->Opener->EndPosition - term->Opener->StartPosition + 1;
      for (int x = term->SubTerms.size() - 1; x >= 0; x--)
        term->SubTerms[x]->TranslateTerm(-odiff);
      // (adjustment of other terms' start / end positions is not necessary
      // thanks the internal order of the terms and the back-to-front-order
      // in processing them)
    }
    else // delete from text
    {
      // delete from text
      text.erase(term->Opener->StartPosition,
          term->Closer->EndPosition - term->Opener->StartPosition + 1);

      // (adjustment of other terms' start / end positions is not necessary
      // thanks the internal order of the terms and the back-to-front-order
      // in processing them)

      // remove sub terms automatically
      for (int x = term->SubTerms.size() - 1; x >= 0; x--)
        delete term->SubTerms[x];
      term->SubTerms.clear();
    }
  }

  for (int x = term->SubTerms.size() - 1; x >= 0; x--)
  {
    if (!EvaluateAndRemoveDependencyTerms(term->SubTerms[x], text))
    {
      delete term->SubTerms[x];
      term->SubTerms.erase(term->SubTerms.begin() + x);
    }
  }

  return remain;
}

bool
GenericTextFormatter
::FormatTextInternally(std::string &text)
{
  // (A) first check the dependency terms and remove the terms which are
  // currently not fulfilled

  EvaluateAndRemoveDependencyTerms(m_DependencyTerms, text);

  // (B) format the pure variables and apply modifiers to them

  // prepare some modifiers
  VariableContentModifier vcm;
  vcm.VariablesMap = &m_Variables;
  TrimModifier trimmod;
  UpperCaseModifier ucmod;
  LowerCaseModifier lcmod;
  IntegerFormatModifier intmod;
  FloatingPointFormatModifier floatmod;
  StringFormatModifier strmod;

  // simply walk from left to right and collect the modifiers and the variables
  // as well:
  std::string::size_type p = -1;
  std::string::size_type pc = -1;
  std::string arg;
  std::vector<Modifier *> currentModifiers;
  bool argContentEmpty = false;
  currentModifiers.clear();
  do
  {
    if (argContentEmpty) // variable-content in previous run was empty
    {
      argContentEmpty = false; // set back
      p--; // only if empty because "${"-based injections cause inf. loops!
    }

    p = text.find("${", p + 1);
    pc = text.find("}", p + 1);

    if (p != std::string::npos && pc != std::string::npos && p < pc)
    {
      arg = text.substr(p + 2, pc - p - 2);
      bool isVar = false;
      if (arg == "/trim")
        currentModifiers.push_back(&trimmod);
      else if (arg == "/ucase")
        currentModifiers.push_back(&ucmod);
      else if (arg == "/lcase")
        currentModifiers.push_back(&lcmod);
      else if (arg.substr(0, 9) == "/iformat:")
      {
        intmod.SetArgumentsFromPureModifierString(arg);
        currentModifiers.push_back(&intmod);
      }
      else if (arg.substr(0, 9) == "/fformat:")
      {
        floatmod.SetArgumentsFromPureModifierString(arg);
        currentModifiers.push_back(&floatmod);
      }
      else if (arg.substr(0, 9) == "/sformat:")
      {
        strmod.SetArgumentsFromPureModifierString(arg);
        currentModifiers.push_back(&strmod);
      }
      else
        isVar = true;

      if (isVar)
      {
        // first apply the content replacer modifier
        arg = vcm.ApplyModifierToString(arg);
        argContentEmpty = (arg.length() == 0);
        // apply the optional (collected) modifiers
        for (unsigned int i = 0; i < currentModifiers.size(); i++)
          arg = currentModifiers[i]->ApplyModifierToString(arg);
        // replace string in text
        text.replace(p, pc - p + 1, arg.c_str(), arg.length());
        currentModifiers.clear(); // set back
      }
      else // modifier
      {
        // simply clean text
        text.erase(p, pc - p + 1);
        p--;
      }
    }
  } while (p != std::string::npos);

  return true;
}

bool
GenericTextFormatter
::FormatText(std::string &text)
{
  if (CheckText(text))
    return FormatTextInternally(text);

  return false;
}

std::string
GenericTextFormatter
::FormatTextF(std::string &text)
{
  std::string s = text;
  if (!FormatText(s))
    s = "";
  return s;
}

bool
GenericTextFormatter
::CheckText(std::string &text)
{
  m_LastErrorMessage = ""; // initialize ERRORS

  // (A) DEPENDENCY TERM RELATED CHECKS AND PREPROCESSING

  // scan the dependency terms:
  std::string::size_type p1 = -1;
  std::string::size_type p2 = -1;
  std::string::size_type p;
  std::string::size_type pc;
  std::string::size_type pc2;
  std::string::size_type p1t;
  std::string::size_type p2t;
  std::string argument;
  bool opening = true;
  std::vector<DependencyTermPart *> dtparts;
  do
  {
    p1t = text.find("${d:", p1 + 1);
    p2t = text.find("${/d:", p2 + 1);
    p = std::string::npos;
    argument = "";
    if (p1t != std::string::npos && p2t != std::string::npos)
    {
      if (p1t < p2t) // the earlier one!
      {
        p = p1t;
        opening = true;
      }
      else
      {
        opening = false;
        p = p2t;
      }
    }
    else if (p1t != std::string::npos)
    {
      p = p1t;
      opening = true;
    }
    else if (p2t != std::string::npos)
    {
      p = p2t;
      opening = false;
    }

    if (p != std::string::npos) // search for "}"
    {
      pc = text.find('}', p + 1);
      pc2 = text.find("${", p + 1);
      if (pc != std::string::npos &&
          (pc2 == std::string::npos || pc < pc2))
      {
        argument = text.substr(p + 4, pc - p - 4);
      }
      else // invalid: set back
      {
        std::ostringstream os;
        os << "SYNTAX-ERROR: a dependency term '${d:' misses its terminating" <<
          " '}' (invalid term starts @ POSITON " << p << " in text).";
        m_LastErrorMessage = os.str();
        p = std::string::npos;
      }
    }

    if (p != std::string::npos) // seems to be OK
    {
      if (opening) // need to check argument -> build tree
      {
        if (BuildLogicalTreeTermFromExpression(argument))
        {
          DependencyTermPart *part = new DependencyTermPart();
          part->Opener = true;
          part->StartPosition = p;
          part->EndPosition = pc;
          part->Criterion = m_LogicalTreeTerm;
          m_LogicalTreeTerm = NULL;
          dtparts.push_back(part);
          p1 = p;
        }
        else
          p = std::string::npos; // error message is internally written
      }
      else // closer
      {
        DependencyTermPart *part = new DependencyTermPart();
        part->Opener = false;
        part->StartPosition = p;
        part->EndPosition = pc;
        part->Criterion = NULL;
        dtparts.push_back(part);
        p2 = p;
      }
    }
  } while (p != std::string::npos);

  // check nesting of dependency terms:
  int integrity = 0;
  for (unsigned int i = 0; i < dtparts.size(); i++) // order is authentical
  {
    if (dtparts[i]->Opener)
      integrity++;
    else
      integrity--;
    if (integrity < 0)
    {
      std::ostringstream os;
      os << "NESTING-ERROR: the nesting of dependency terms seems to be " <<
          "invalid (@ POSITION " << dtparts[i]->StartPosition << ").";
      m_LastErrorMessage = os.str();
      break;
    }
  }
  if (integrity > 0)
  {
    std::ostringstream os;
    os << "NESTING-ERROR: the nesting of dependency terms seems to be " <<
        "invalid. " << integrity << " dependency term(s) not closed.";
    m_LastErrorMessage = os.str();
  }

  if (m_LastErrorMessage.length() <= 0) // no error
  {
    // create dependency terms:
    if (m_DependencyTerms)
    {
      delete m_DependencyTerms;
      m_DependencyTerms = NULL;
    }
    m_DependencyTerms = new DependencyTerm(); // prepare root term
    m_DependencyTerms->Opener = NULL; // root term has no opener / closer
    m_DependencyTerms->Closer = NULL;
    m_DependencyTerms->SubTerms.clear(); // ... just sub-terms!
    m_DependencyTerms->Level = -1; // root level
    std::vector<DependencyTermPart *> openers; // ordered opener parts
    std::vector<DependencyTerm *> qterms; // queued terms
    integrity = 0;
    for (unsigned int i = 0; i < dtparts.size(); i++) // order is authentical
    {
      if (dtparts[i]->Opener) // -> opens a new term
      {
        openers.push_back(dtparts[i]); // queue it
        integrity++;
      }
      else // -> closes current term
      {
        integrity--;
        DependencyTerm *term = new DependencyTerm();
        term->Closer = dtparts[i];
        term->Opener = openers[integrity];
        term->SubTerms.clear();
        term->Level = integrity;
        openers.pop_back();

        if (integrity == 0) // zero-level is OK (root term exists surely)
        {
          m_DependencyTerms->SubTerms.push_back(term);
        }
        else // terms on other levels must be queued temporarily
        {
          qterms.push_back(term);
        }
      }
    }
    // process nested terms and insert them into the right parent terms:
    int lev = 1;
    bool couldInsert;
    std::vector<DependencyTerm *> flatterms; // inserted terms in flat style
    for (unsigned int y = 0; y < m_DependencyTerms->SubTerms.size(); y++)
      flatterms.push_back(m_DependencyTerms->SubTerms[y]);
    do
    {
      couldInsert = false;
      for (int x = qterms.size() - 1; x >= 0; x--)
      {
        if (qterms[x]->Level == lev)
        {
          bool itemCouldInsert = false;
          // -> search for the parent term including this one:
          for (unsigned int y = 0; y < flatterms.size(); y++)
          {
            if (flatterms[y]->Level == (lev - 1))
            {
              if (flatterms[y]->Opener->StartPosition <
                  qterms[x]->Opener->StartPosition &&
                  flatterms[y]->Closer->EndPosition >
                  qterms[x]->Closer->EndPosition)
              {
                // insert in parent (at first position - results from ordering
                // within qterms):
                flatterms[y]->SubTerms.insert(flatterms[y]->SubTerms.begin(),
                    qterms[x]);
                flatterms.push_back(qterms[x]); // add pointer for flat search
                qterms.erase(qterms.begin() + x);
                couldInsert = true;
                itemCouldInsert = true;
                break;
              }
            }
          }
          if (!itemCouldInsert)
          {
            std::ostringstream os;
            os << "NESTING-ERROR: at least one nested dependency term " <<
              "could not be inserted (start position @ " <<
              qterms[x]->Opener->StartPosition << ", close position @ " <<
              qterms[x]->Closer->EndPosition << ").";
            m_LastErrorMessage = os.str();
            break;
          }
        }
      }
      lev++;
    } while (qterms.size() > 0 && couldInsert &&
             m_LastErrorMessage.length() == 0);
    flatterms.clear(); // just pointers

    if (m_LastErrorMessage.length() > 0) // error occurred -> clean up
    {
      for (unsigned int i = 0; i < qterms.size(); i++)
        delete qterms[i];
      qterms.clear();
    }
  }
  else
  {
    // clean up
    for (unsigned int i = 0; i < dtparts.size(); i++)
      delete dtparts[i];
    dtparts.clear();
  }


  // (B) VARIABLE EXPRESSION RELATED CHECKS AND PREPROCESSING

  if (m_LastErrorMessage.length() == 0)
  {
    bool validated = false;
    p = -1;
    do
    {
      p = text.find("${", p + 1);
      if (p != std::string::npos)
      {
        if (text.substr(p, 4) != "${d:" &&
            text.substr(p, 5) != "${/d:") // no dep. term related expression
        {
          pc = text.find('}', p + 1);
          pc2 = text.find("${", p + 1);
          if (pc != std::string::npos &&
              (pc2 == std::string::npos || pc < pc2))
          {
            argument = text.substr(p + 2, pc - p - 2);

            // simple modifiers:
            // ${/trim}
            // ${/ucase}
            // ${/lcase}
            validated = false;
            if (argument == "/trim" ||
                argument == "/ucase" ||
                argument == "/lcase")
              validated = true;
            // advanced modifiers (with argument):
            // ${/iformat:format-string}
            // ${/sformat:format-string}
            // ${/fformat:format-string}
            if (!validated)
            {
              if (argument.substr(0, 9) == "/iformat:" &&
                  argument.length() > 9)
                validated = true;
              if (argument.substr(0, 9) == "/sformat:" &&
                  argument.length() > 9)
                validated = true;
              if (argument.substr(0, 9) == "/fformat:" &&
                  argument.length() > 9)
                validated = true;
            }
            // ordinary variables:
            // ${VARNAME}
            if (!validated)
            {
              // try to validate the variable
              std::map<std::string, VariableMetaData *>::iterator it =
                m_Variables.find(argument);
              if (it == m_Variables.end())
              {
                std::ostringstream os;
                os << "VARIABLE TERM ERROR: the variable '" << argument <<
                  "' could not be found in internal variable list.";
                m_LastErrorMessage = os.str();
                p = std::string::npos;
              }
            }
          }
          else // invalid: set back
          {
            std::ostringstream os;
            os << "SYNTAX-ERROR: a variable term '${' misses its terminating" <<
              " '}' (invalid term starts @ POSITON " << p << " in text).";
            m_LastErrorMessage = os.str();
            p = std::string::npos;
          }
        }
      }
    } while (p != std::string::npos);
  }

  // clean up
  if (m_LastErrorMessage.length() > 0) // error occurred -> clean up
    CleanUp();

  return (m_LastErrorMessage.length() <= 0);
}

bool
GenericTextFormatter
::BuildLogicalTreeTermFromExpression(std::string exp)
{
  if (m_LogicalTreeTerm)
  {
    delete m_LogicalTreeTerm;
    m_LogicalTreeTerm = NULL;
  }

  std::vector<std::string> ors;
  TokenizeIncludingEmptySpaces(exp, ors, "|");
  if (ors.size() <= 0)
    ors.push_back(exp); // no OR

  std::vector<LogicalTreeTerm *> orOperands;
  for (unsigned int i = 0; i < ors.size(); i++)
  {
    std::string s = TrimF(ors[i]);
    if (s.length() == 0)
    {
      std::ostringstream os;
      os << "LOGICAL TERM ERROR: an OR-related operand (operand number: " <<
        (i + 1) << ") in expression '" << exp <<
        "' is empty. OR is a binary operator!";
      m_LastErrorMessage = os.str();
      return false;
    }

    std::vector<std::string> ands;
    TokenizeIncludingEmptySpaces(s, ands, "&");
    if (ands.size() <= 0)
      ands.push_back(s); // no AND
    std::vector<Operand *> andOperands;
    for (unsigned int j = 0; j < ands.size(); j++)
    {
      std::string s2 = TrimF(ands[j]);
      if (s2.length() == 0)
      {
        std::ostringstream os;
        os << "LOGICAL TERM ERROR: an AND-related operand (operand number: " <<
          (j + 1) << ") within expression '" << s <<
          "' is empty. AND is a binary operator!";
        m_LastErrorMessage = os.str();
        return false;
      }

      Operand *op = new Operand();
      if (!op->ApplyPropertiesFromOperandString(s2, &m_Variables))
      {
        std::ostringstream os;
        os << "LOGICAL TERM ERROR: an AND-related operand (operand number: " <<
          (j + 1) << ", " << s2 << ") within expression '" << s <<
          "' could not be translated into a valid operand type.";
        m_LastErrorMessage = os.str();
        return false;
      }

      andOperands.push_back(op);
    }

    LogicalTreeTerm *andTerm = new LogicalTreeTerm();
    andTerm->ANDOperationType = true;
    for (unsigned int k = 0; k < andOperands.size(); k++)
      andTerm->Operands.push_back(andOperands[k]);
    orOperands.push_back(andTerm);
  }

  if (orOperands.size() > 0)
  {
    m_LogicalTreeTerm = new LogicalTreeTerm();
    m_LogicalTreeTerm->ANDOperationType = false;
    for (unsigned int k = 0; k < orOperands.size(); k++)
      m_LogicalTreeTerm->SubTerms.push_back(orOperands[k]);
  }
  else // just an AND-operation
  {
    m_LogicalTreeTerm = orOperands[0];
  }
  orOperands.clear();

  return true;
}

void
GenericTextFormatter
::CleanUp()
{
  if (m_LogicalTreeTerm)
  {
    delete m_LogicalTreeTerm;
    m_LogicalTreeTerm = NULL;
  }
  if (m_DependencyTerms)
  {
    delete m_DependencyTerms;
    m_DependencyTerms = NULL;
  }
}


}
