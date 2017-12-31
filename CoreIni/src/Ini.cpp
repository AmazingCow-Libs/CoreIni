
// Header
#include "../include/Ini.h"
// std
#include <algorithm>
#include <iterator>
// Amazing Cow Libs
#include "CoreAssert/CoreAssert.h"
#include "CoreFS/CoreFS.h"
#include "CoreFile/CoreFile.h"
#include "CoreString/CoreString.h"


// Usings
USING_NS_COREINI;


//----------------------------------------------------------------------------//
// Section                                                                    //
//----------------------------------------------------------------------------//
const char* Ini::Section::KGlobalName = "CoreIni::Global::Section";



//----------------------------------------------------------------------------//
// CTOR / DTOR                                                                //
//----------------------------------------------------------------------------//
Ini::Ini(
    const std::string &filename,
    unsigned int       commentType        /* = INI_COMMENT_DEFAULT               */,
    unsigned int       duplicateMode      /* = INI_DUPLICATE_MERGE_AND_OVERWRITE */,
    bool               allowQuoted        /* = true                              */,
    bool               allowBackslashes   /* = true                              */,
    bool               allowGlobals       /* = true                              */,
    bool               allowHierarchy     /* = true                              */,
    char               hierarchyDelimiter /* = '/'                               */,
    char               keyValueDelimiter  /* = '='                               */) :
    // Members
    m_commentType       (       commentType),
    m_duplicateMode     (     duplicateMode),
    m_allowQuoted       (       allowQuoted),
    m_allowBackslashes  (  allowBackslashes),
    m_allowGlobals      (      allowGlobals),
    m_allowHierarchy    (    allowHierarchy),
    m_hierarchyDelimiter(hierarchyDelimiter),
    m_keyValueDelimiter ( keyValueDelimiter)
{
    COREASSERT_THROW_IF(
        !CoreFS::IsFile(filename),
        std::invalid_argument,
        "File doesn't exists - filename: (%s)",
        filename.c_str()
    );

    Parse(CoreFile::ReadAllLines(filename));
}

//----------------------------------------------------------------------------//
// Section                                                                    //
//----------------------------------------------------------------------------//
const std::vector<Ini::Section>& Ini::GetSections() const noexcept
{
    return m_sections;
}

std::vector<std::string> Ini::GetSectionNames() const noexcept
{
    auto names = std::vector<std::string>();
    names.reserve(m_sections.size());

    // Push all names into vector.
    std::for_each(
        std::begin(m_sections),
        std::end  (m_sections),
        [&names](const Section &section){
            names.push_back(section.name);
        }
    );

    return names;
}

const Ini::Section& Ini::GetSection(const std::string &path) const
{
    auto it = std::find_if(
        std::begin(m_sections),
        std::end  (m_sections),
        [&path](const Section &section) { return section.name == path; }
    );

    COREASSERT_THROW_IF(
        it == std::end(m_sections),
        std::invalid_argument,
        "Section doesn't exists - path: (%s)",
        path.c_str()
    );

    return *it;
}

bool Ini::SectionExists(const std::string &path) const noexcept
{
    auto it = std::find_if(
        std::begin(m_sections),
        std::end  (m_sections),
        [&path](const Section &section) { return section.name == path; }
    );

    return it != std::end(m_sections);
}


//----------------------------------------------------------------------------//
// Values                                                                     //
//----------------------------------------------------------------------------//
const Ini::Value& Ini::GetValue(
    const Section     &section,
    const std::string &name)
{
    auto it = std::find_if(
        std::begin(section.values),
        std::end  (section.values),
        [&name](const Value &value) { return value.name == name; }
    );

    COREASSERT_THROW_IF(
        it == std::end(section.values),
        std::invalid_argument,
        "There's no value with name: (%s) in section: (%s)",
        name.c_str(),
        section.name.c_str()
    );

    return *it;
}

const Ini::Value& Ini::GetValue(
    const std::string &sectionName,
    const std::string &name)
{
    return GetValue(GetSection(sectionName), name);
}

bool Ini::ValueExists(
    const Section     &section,
    const std::string &name) const noexcept
{
    return ValueExists(section.name, name);
}

bool Ini::ValueExists(
    const std::string &sectionName,
    const std::string &name) const noexcept
{
    // Section doesn't exists, so the value.
    if(!SectionExists(sectionName))
        return false;

    const auto &section = GetSection(sectionName);
    auto it = std::find_if(
        std::begin(section.values),
        std::end  (section.values),
        [&name](const Value &value) { return value.name == name; }
    );

    return it != std::end(section.values);
}


// Private Methods
void Ini::Parse(const std::vector<std::string> &lines)
{
    auto clean_lines = std::vector<std::string>();
    clean_lines.reserve(lines.size());

    Section *p_curr_section = nullptr;
    auto section_name = std::string();
    auto key_value    = std::vector<std::string>();

    for(const auto &line : lines)
    {
        //----------------------------------------------------------------------
        // Empty or comments... - Ignore those.
        if(IsCommentLine(line))
            continue;
        if(CoreString::IsNullOrWhiteSpace(line))
            continue;

        //----------------------------------------------------------------------
        // Section.
        if(IsSectionLine(line, &section_name))
        {
            if(SectionExists(section_name))
            {
                p_curr_section = const_cast<Section *>(&GetSection(section_name));
            }
            else
            {
                auto section = Section{
                    .name = section_name
                };
                m_sections.push_back(section);
                p_curr_section = &m_sections.back();
            }

            continue;
        }

        //----------------------------------------------------------------------
        // Value
        if(IsValueLine(line, &key_value))
        {
            //------------------------------------------------------------------
            // We're dealing with a global value and we allow globals values,
            // but haven't yet a global section, so let's create it.
            if(m_allowGlobals && !p_curr_section)
            {
                auto global_section = Section {
                    .name = Section::KGlobalName
                };

                m_sections.push_back(global_section);
                p_curr_section = &m_sections.back();
            }
            //------------------------------------------------------------------
            // We're dealing with a global value, but we don't allow it.
            else if(!m_allowGlobals && !p_curr_section)
            {
                auto msg = CoreString::Format(
                    "Found a global value but CoreIni is set to not allow them - Line: (%s)",
                    line
                );

                throw std::logic_error(msg);
            }

            //------------------------------------------------------------------
            auto exists = ValueExists(p_curr_section->name, key_value[0]);
            // Disallow any duplicates.
            if(exists && m_duplicateMode == INI_DUPLICATE_DISALLOW)
            {
                auto msg = CoreString::Format(
                    "Value is duplicated but CoreIni is set to not allow them - Line: (%s)",
                    line
                );

                throw std::logic_error(msg);
            }
            // Ignore any duplicates.
            else if(exists && m_duplicateMode == INI_DUPLICATE_MERGE_AND_IGNORE)
            {
                // Just ignore...
                continue;
            }
            // Overwrite any duplicates.
            else if(exists && (m_duplicateMode == INI_DUPLICATE_OVERWRITE ||
                               m_duplicateMode == INI_DUPLICATE_MERGE_AND_OVERWRITE))
            {
                // COWTODO(n2omatt): How to replace??
            }
            // Doesn't exits, just add.
            else
            {
                auto val = Value {
                    .name  = key_value[0],
                    .value = key_value[1]
                };

                p_curr_section->values.push_back(val);
            }

            continue;
        } // if(IsValueLine(line, &key_value))
    } // for(const auto &line : lines)
}

bool Ini::IsCommentLine(const std::string &line) const noexcept
{
    // Empty lines aren't considered comments.
    if(line.empty())
        return false;

    // We don't care for leading empty chars.
    auto clean = CoreString::TrimStart(line);
    if(clean.empty())
        return false;

    // Semicolon ;
    if(clean.front() == ';' && (m_commentType & INI_COMMENT_SEMICOLON))
        return true;

    // Hash #
    if(clean.front() == '#' && (m_commentType & INI_COMMENT_HASH))
        return true;

    // COWTODO(n2omatt): What we gonna do with INI_COMMENT_NONE???

    return false;
}

bool Ini::IsSectionLine(
    const std::string &line,
    std::string       *pOut_SectionName) const noexcept
{
    COREASSERT_ASSERT(pOut_SectionName, "pOut_SectionName can't be nullptr");

    //COWTODO(n2omatt): Use regex???
    auto clean_name  = CoreString::Trim(StripComments(line));

    // Empty string - Can't be section name.
    if(clean_name.empty())
        return false;

    // Section name must be inside brackets [, ]
    if(clean_name.front() == '[' && clean_name.back() == ']')
    {
        *pOut_SectionName = std::string(
            std::begin(clean_name) +1,
            std::end  (clean_name) -1
        );
        return true;
    }

    // Doesn't matches the requirement of be inside brackets [, ]
    return false;
}

bool Ini::IsValueLine(
    const std::string &line,
    std::vector<std::string> *pOut_KeyVal) const noexcept
{
    COREASSERT_ASSERT(pOut_KeyVal, "pOut_KeyVal can't be nullptr");

    //COWTODO(n2omatt): Use regex???
    auto clean_line = CoreString::Trim(StripComments(line));

    if(clean_line.empty())
        return false;

    auto key_val = CoreString::Split(
        clean_line,
        m_keyValueDelimiter
    );

    // A property must be of size 2.
    if(key_val.size() != 2)
        return false;

    auto key   = CoreString::Trim(key_val[0]);
    auto value = CoreString::Trim(key_val[1]);

    // We must have both components of the property.
    if(key.empty() || value.empty())
        return false;

    // Make sure that we have the correct size.
    if(pOut_KeyVal->size() != 2)
        pOut_KeyVal->resize(2);

    // Copy...
    (*pOut_KeyVal)[0] = key;
    (*pOut_KeyVal)[1] = value;

    return true;
}

std::string Ini::StripComments(const std::string &line) const noexcept
{
    if(m_commentType == INI_COMMENT_NONE)
        return line;

    std::string comment_chars;
    if(m_commentType & INI_COMMENT_HASH     ) comment_chars += "#";
    if(m_commentType & INI_COMMENT_SEMICOLON) comment_chars += ";";

    auto first_index = CoreString::IndexOfAny(line, comment_chars);
    return line.substr(0, first_index);
}
