//~---------------------------------------------------------------------------//
//                     _______  _______  _______  _     _                     //
//                    |   _   ||       ||       || | _ | |                    //
//                    |  |_|  ||       ||   _   || || || |                    //
//                    |       ||       ||  | |  ||       |                    //
//                    |       ||      _||  |_|  ||       |                    //
//                    |   _   ||     |_ |       ||   _   |                    //
//                    |__| |__||_______||_______||__| |__|                    //
//                             www.amazingcow.com                             //
//  File      : Ini.cpp                                                       //
//  Project   : CoreIni                                                       //
//  Date      : Dec 23, 2017                                                  //
//  License   : GPLv3                                                         //
//  Author    : n2omatt <n2omatt@amazingcow.com>                              //
//  Copyright : AmazingCow - 2017, 2018                                       //
//                                                                            //
//  Description :                                                             //
//---------------------------------------------------------------------------~//

// Header
#include "../include/Ini.h"
// std
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <sstream>
// Amazing Cow Libs
#include "acow/cpp_goodies.h"
#include "CoreAssert/CoreAssert.h"
#include "CoreFS/CoreFS.h"
#include "CoreFile/CoreFile.h"
#include "CoreString/CoreString.h"

// Usings
USING_NS_COREINI;


//----------------------------------------------------------------------------//
// Macros                                                                     //
//----------------------------------------------------------------------------//
#define INI_THROW_IF(_cond_, _exception_, _fmt_, ...)                     \
    do {                                                                  \
        if((_cond_))                                                     \
            throw _exception_(CoreString::Format(_fmt_,  ##__VA_ARGS__)); \
    } while(0)


//----------------------------------------------------------------------------//
// Section                                                                    //
//----------------------------------------------------------------------------//
const char* Section::kGlobalName = "CoreIni::Global::Section";


//----------------------------------------------------------------------------//
// CTOR / DTOR                                                                //
//----------------------------------------------------------------------------//
Ini::Ini(
    const std::string &filename,
    uint8_t            commentType,          /* = INI_COMMENT_DEFAULT    */
    uint8_t            sectionDuplicateMode, /* = INI_DUPLICATE_DISALLOW */
    uint8_t            valueDuplicateMode,   /* = INI_DUPLICATE_DISALLOW */
    bool               allowQuoted,          /* = true                   */
    bool               allowBackslashes,     /* = true                   */
    bool               allowGlobals,         /* = true                   */
    bool               allowHierarchy,       /* = true                   */
    char               hierarchyDelimiter,   /* = '/'                    */
    char               keyValueDelimiter)    /* = '='                    */
    // Members
    : m_commentType         (         commentType)
    , m_sectionDuplicateMode(sectionDuplicateMode)
    , m_valueDuplicateMode  (  valueDuplicateMode)
    , m_allowQuoted         (         allowQuoted)
    , m_allowBackslashes    (    allowBackslashes)
    , m_allowGlobals        (        allowGlobals)
    , m_allowHierarchy      (      allowHierarchy)
    , m_hierarchyDelimiter  (  hierarchyDelimiter)
    , m_keyValueDelimiter   (   keyValueDelimiter)
{
    //--------------------------------------------------------------------------
    // Sanity checks...
    INI_THROW_IF(
        !CoreFS::IsFile(filename),
        std::invalid_argument,
        "File doesn't exists - filename: (%s)",
        filename.c_str()
    );

    //--------------------------------------------------------------------------
    // Parse the file.
    Parse(CoreFile::ReadAllLines(filename));
}

Ini::Ini(
    uint8_t            commentType,          /* = INI_COMMENT_DEFAULT    */
    uint8_t            sectionDuplicateMode, /* = INI_DUPLICATE_DISALLOW */
    uint8_t            valueDuplicateMode,   /* = INI_DUPLICATE_DISALLOW */
    bool               allowQuoted,          /* = true                   */
    bool               allowBackslashes,     /* = true                   */
    bool               allowGlobals,         /* = true                   */
    bool               allowHierarchy,       /* = true                   */
    char               hierarchyDelimiter,   /* = '/'                    */
    char               keyValueDelimiter)    /* = '='                    */
    // Members
    : m_commentType         (         commentType)
    , m_sectionDuplicateMode(sectionDuplicateMode)
    , m_valueDuplicateMode  (  valueDuplicateMode)
    , m_allowQuoted         (         allowQuoted)
    , m_allowBackslashes    (    allowBackslashes)
    , m_allowGlobals        (        allowGlobals)
    , m_allowHierarchy      (      allowHierarchy)
    , m_hierarchyDelimiter  (  hierarchyDelimiter)
    , m_keyValueDelimiter   (   keyValueDelimiter)
{
    // Empty...
}

void Ini::Save(const std::string &path)
{
    std::stringstream ss;
    for(const auto &section : m_sections)
    {
        ss << CoreString::Format("[%s]\n", section.m_name);
        for(const auto &value : section.m_values)
        {
            ss << CoreString::Format(
                "    %s %c %s\n",
                value.m_name,
                m_keyValueDelimiter,
                value.m_content
            );
        }
        ss << "\n";
    }

    CoreFile::WriteAllText(CoreFS::ExpandUserAndMakeAbs(path), ss.str());
}


//----------------------------------------------------------------------------//
// Add Section                                                                //
//----------------------------------------------------------------------------//
void Ini::AddSection(const std::string &sectionName)
{
    auto section_exists = SectionExists(sectionName);

    //--------------------------------------------------------------------------
    // Ignore Mode - Just return if already exists.
    if(section_exists && ACOW_FLAG_HAS(INI_DUPLICATE_IGNORE, m_sectionDuplicateMode))
        return;

    //--------------------------------------------------------------------------
    // Disallow mode - Always throw if section exits..
    INI_THROW_IF(
        section_exists && ACOW_FLAG_HAS(INI_DUPLICATE_DISALLOW, m_sectionDuplicateMode),
        std::invalid_argument,
        "Section: (%s) already exists.",
        sectionName.c_str()
    );

    //--------------------------------------------------------------------------
    // Overwrite mode - Clear previous or add a new if needed.
    if(ACOW_FLAG_HAS(INI_DUPLICATE_OVERWRITE, m_sectionDuplicateMode))
    {
        if(section_exists)
        {
            auto &section = GetSection(sectionName);
            const_cast<Section &>(section).m_values.clear();
        }
        else
        {
            m_sections.push_back(Section(sectionName));
        }

        return;
    }

    //--------------------------------------------------------------------------
    // Merge mode - But section.
    if(ACOW_FLAG_HAS(INI_DUPLICATE_MERGE, m_sectionDuplicateMode))
    {
        if(!section_exists)
            m_sections.push_back(Section(sectionName));

        return;
    }

    m_sections.push_back(Section(sectionName));
}


//----------------------------------------------------------------------------//
// Remove Section                                                             //
//----------------------------------------------------------------------------//
void Ini::RemoveSection(const std::string &name)
{
    INI_THROW_IF(
        SectionExists(name),
        std::invalid_argument,
        "Section: (%s) doesn't exists",
        name.c_str()
    );

    m_sections.erase(std::remove_if(
        std::begin(m_sections),
        std::end  (m_sections),
        [&name](const Section &s) {
            return s.m_name == name;
        }
    ));
}



//----------------------------------------------------------------------------//
// Get Section                                                                //
//----------------------------------------------------------------------------//
const Section& Ini::GetSection(const std::string &path) const
{
    auto it = std::find_if(
        std::begin(m_sections),
        std::end  (m_sections),
        [&path](const Section &section) {
            return section.m_name == path;
        }
    );

    INI_THROW_IF(
        it == std::end(m_sections),
        std::invalid_argument,
        "Section doesn't exists - path: (%s)",
        path.c_str()
    );

    return *it;
}

const std::vector<Section>& Ini::GetSections() const noexcept
{
    return m_sections;
}

std::vector<std::string> Ini::GetSectionNames() const noexcept
{
    auto names = std::vector<std::string>();
    names.reserve(m_sections.size());

    //--------------------------------------------------------------------------
    // Push all names into vector.
    std::for_each(
        std::begin(m_sections),
        std::end  (m_sections),
        [&names](const Section &section){
            names.push_back(section.m_name);
        }
    );

    return names;
}


bool Ini::SectionExists(const std::string &path) const noexcept
{
    auto it = std::find_if(
        std::begin(m_sections),
        std::end  (m_sections),
        [&path](const Section &section) {
            return section.m_name == path;
        }
    );

    return it != std::end(m_sections);
}

//----------------------------------------------------------------------------//
// Add Value                                                                  //
//----------------------------------------------------------------------------//
void Ini::AddValue(
    const std::string &sectionName,
    const std::string &valueName,
    const std::string &valueContent)
{
    auto value_exists = ValueExists(sectionName, valueName);

    //--------------------------------------------------------------------------
    // Ignore mode - Just return if needed.
    if(value_exists && ACOW_FLAG_HAS(INI_DUPLICATE_IGNORE, m_valueDuplicateMode))
        return;

    //--------------------------------------------------------------------------
    // Disallow mode - Always throws if needed.
    INI_THROW_IF(
        value_exists && ACOW_FLAG_HAS(INI_DUPLICATE_DISALLOW, m_valueDuplicateMode),
        std::invalid_argument,
        "Section: (%s)'s Value: (%s) already exists.",
        sectionName.c_str(),
        valueName  .c_str()
    );

    //--------------------------------------------------------------------------
    // Overwrite Mode.
    if(value_exists)
    {
        auto &value = GetValue(sectionName, valueName);
        const_cast<Value &>(value).m_content = valueContent;
    }
    else
    {
        const_cast<Section &>(
            GetSection(sectionName)
        ).m_values.push_back(Value(valueName, valueContent));
    }
}

//----------------------------------------------------------------------------//
// Remove Value                                                               //
//----------------------------------------------------------------------------//
void Ini::RemoveValue(
    const std::string &sectionName,
    const std::string &valueName)
{
    //--------------------------------------------------------------------------
    // Check if values exists.
    auto value_exists = ValueExists(sectionName, valueName);
    INI_THROW_IF(
        !value_exists,
        std::invalid_argument,
        "Section: (%s) - Value: (%s) doesn't exits.",
        sectionName.c_str(),
        valueName  .c_str()
    );

    auto &section = GetSection(sectionName);
    auto &values  = const_cast<Section &>(section).m_values;
    values.erase(
        std::remove_if(
            std::begin(values),
            std::end  (values),
            [&valueName](const Value &v) {
                return v.m_name == valueName;
            }
        )
    );
}


//----------------------------------------------------------------------------//
// Get Value                                                                  //
//----------------------------------------------------------------------------//
const Value& Ini::GetValue(
    const std::string &sectionName,
    const std::string &valueName) const
{
    auto &section = GetSection(sectionName);
    auto &values  = section.GetValues();

    auto it = std::find_if(
        std::begin(values),
        std::end  (values),
        [&valueName](const Value &v) {
            return v.m_name == valueName;
        }
    );

    INI_THROW_IF(
        it == std::end(values),
        std::invalid_argument,
        "Section (%s) - Value (%s) doesn't exists.",
        sectionName.c_str(),
        valueName  .c_str()
    );

    return *it;
}


bool Ini::ValueExists(
    const std::string &sectionName,
    const std::string &valueName) const noexcept
{
    //--------------------------------------------------------------------------
    // Section doesn't exists, so the value.
    if(!SectionExists(sectionName))
        return false;

    auto &section = GetSection(sectionName);
    auto &values  = section.GetValues();

    auto it = std::find_if(
        std::begin(values),
        std::end  (values),
        [&valueName](const Value &v) {
            return v.m_name == valueName;
        }
    );

    return it != std::end(values);
}

//----------------------------------------------------------------------------//
// Private Methods                                                            //
//----------------------------------------------------------------------------//
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
                auto section = Section(section_name);

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
                auto global_section = Section(Section::kGlobalName);

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

            auto exists = ValueExists(p_curr_section->m_name, key_value[0]);
            //------------------------------------------------------------------
            // Disallow any duplicates.
            if(exists && ACOW_FLAG_HAS(INI_DUPLICATE_DISALLOW, m_valueDuplicateMode))
            {
                auto msg = CoreString::Format(
                    "Value is duplicated but CoreIni is set to not allow them - Line: (%s)",
                    line
                );

                throw std::logic_error(msg);
            }
            //------------------------------------------------------------------
            // Ignore any duplicates.
            else if(exists && ACOW_FLAG_HAS(INI_DUPLICATE_IGNORE, m_valueDuplicateMode))
            {
                // Just ignore...
                continue;
            }
            //------------------------------------------------------------------
            // Overwrite any duplicates.
            else if(exists && ACOW_FLAG_HAS(INI_DUPLICATE_OVERWRITE, m_valueDuplicateMode))
            {
                auto &value = GetValue(section_name, key_value[0]);
                const_cast<Value &>(value).m_content = key_value[1];
            }
            //------------------------------------------------------------------
            // Doesn't exits, just add.
            else
            {
                auto val = Value(key_value[0], key_value[1]);
                p_curr_section->m_values.push_back(val);
            }

            continue;
        } // if(IsValueLine(line, &key_value))
    } // for(const auto &line : lines)
}

bool Ini::IsCommentLine(const std::string &line) const noexcept
{
    //--------------------------------------------------------------------------
    // Empty lines aren't considered comments.
    if(line.empty())
        return false;

    //--------------------------------------------------------------------------
    // We don't care for leading empty chars.
    auto clean = CoreString::TrimStart(line);
    if(clean.empty())
        return false;

    //--------------------------------------------------------------------------
    // Semicolon ;
    if(clean.front() == ';' && ACOW_FLAG_HAS(INI_COMMENT_SEMICOLON, m_commentType))
        return true;

    //--------------------------------------------------------------------------
    // Hash #
    if(clean.front() == '#' && ACOW_FLAG_HAS(INI_COMMENT_HASH, m_commentType))
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

    //--------------------------------------------------------------------------
    // Empty string - Can't be section name.
    if(clean_name.empty())
        return false;

    //--------------------------------------------------------------------------
    // Section name must be inside brackets [, ]
    if(clean_name.front() == '[' && clean_name.back() == ']')
    {
        *pOut_SectionName = std::string(
            std::begin(clean_name) +1,
            std::end  (clean_name) -1
        );
        return true;
    }

    //--------------------------------------------------------------------------
    // Doesn't matches the requirement of be inside brackets [, ]
    return false;
}

bool Ini::IsValueLine(
    const std::string        &line,
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

    //--------------------------------------------------------------------------
    // A property must be of size 2.
    if(key_val.size() != 2)
        return false;

    auto key   = CoreString::Trim(key_val[0]);
    auto value = CoreString::Trim(key_val[1]);

    //--------------------------------------------------------------------------
    // We must have both components of the property.
    if(key.empty() || value.empty())
        return false;

    //--------------------------------------------------------------------------
    // Make sure that we have the correct size.
    if(pOut_KeyVal->size() != 2)
        pOut_KeyVal->resize(2);

    //--------------------------------------------------------------------------
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
