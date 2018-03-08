//~---------------------------------------------------------------------------//
//                     _______  _______  _______  _     _                     //
//                    |   _   ||       ||       || | _ | |                    //
//                    |  |_|  ||       ||   _   || || || |                    //
//                    |       ||       ||  | |  ||       |                    //
//                    |       ||      _||  |_|  ||       |                    //
//                    |   _   ||     |_ |       ||   _   |                    //
//                    |__| |__||_______||_______||__| |__|                    //
//                             www.amazingcow.com                             //
//  File      : Ini.h                                                         //
//  Project   : CoreIni                                                       //
//  Date      : Dec 23, 2017                                                  //
//  License   : GPLv3                                                         //
//  Author    : n2omatt <n2omatt@amazingcow.com>                              //
//  Copyright : AmazingCow - 2017, 2018                                       //
//                                                                            //
//  Description :                                                             //
//---------------------------------------------------------------------------~//

#pragma once
//std
#include <stdexcept>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
// CoreIni
#include "CoreIni_Utils.h"


NS_COREINI_BEGIN

class Ini;


class Value
{
    //------------------------------------------------------------------------//
    // CTOR / DTOR                                                            //
    //------------------------------------------------------------------------//
public:
    inline Value(
        const std::string &name    = "",
        const std::string &content = "") noexcept
        : m_name   (name)
        , m_content(content)
    {
        // Empty...
    }

    //------------------------------------------------------------------------//
    // Public Methods                                                         //
    //------------------------------------------------------------------------//
public:
    inline const std::string& GetName   () const noexcept { return m_name;    }
    inline const std::string& GetContent() const noexcept { return m_content; }


    //------------------------------------------------------------------------//
    // iVars                                                                  //
    //------------------------------------------------------------------------//
private:
    friend class Ini;

    std::string m_name;
    std::string m_content;

}; // class Value


class Section
{
    //------------------------------------------------------------------------//
    // Enums / Constants / Typedefs                                           //
    //------------------------------------------------------------------------//
public:
    const static char* kGlobalName;

    //------------------------------------------------------------------------//
    // CTOR / DTOR                                                            //
    //------------------------------------------------------------------------//
public:
    inline Section(
        const std::string        &name    = "",
        const std::vector<Value> &values = {}) noexcept
        : m_name  (name)
        , m_values(values)
    {
        // Empty...
    }

    //------------------------------------------------------------------------//
    // Public Methods                                                         //
    //------------------------------------------------------------------------//
public:
    inline const std::string       & GetName  () const noexcept { return m_name;   }
    inline const std::vector<Value>& GetValues() const noexcept { return m_values; }

    //------------------------------------------------------------------------//
    // iVars                                                                  //
    //------------------------------------------------------------------------//
private:
    friend class Ini;

    std::string        m_name;
    std::vector<Value> m_values;

}; // class Section;


class Ini
{
    //------------------------------------------------------------------------//
    // Enums / Constants / Typedefs                                           //
    //------------------------------------------------------------------------//
public:
    //--------------------------------------------------------------------------
    // Comment type.
    enum {
        INI_COMMENT_SEMICOLON = 1 << 0,
        INI_COMMENT_HASH      = 1 << 1,
        INT_COMMENT_ALL       = INI_COMMENT_HASH | INI_COMMENT_SEMICOLON,
        INI_COMMENT_DEFAULT   = INT_COMMENT_ALL,
        INI_COMMENT_NONE      = 0
    }; // Comment type.

    //--------------------------------------------------------------------------
    // Notice about enums:
    //   The first enum must be a "naked" enum type because we want that
    //   bitwise logic easily implemented, but that enum is the only one
    //   that needs to be that way. ALl other enums could be better written
    //   as the new enum class types, but in sake of the consistency of
    //   the library, all the enums of this class will be "naked ones."
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    // Duplicate mode.
    enum {
        INI_DUPLICATE_DISALLOW,
        INI_DUPLICATE_OVERWRITE,
        INI_DUPLICATE_IGNORE,
        INI_DUPLICATE_MERGE
    }; // Duplicate mode.


    //------------------------------------------------------------------------//
    // CTOR / DTOR                                                            //
    //------------------------------------------------------------------------//
public:
    ///-------------------------------------------------------------------------
    /// @brief
    ///   Constructs a Ini object.
    /// @param commentType
    ///   Which chars will be considered as comments.
    ///   Default: INI_COMMENT_DEFAULT
    /// @param duplicateMode
    ///   How sections and values are handled when duplicates are found.
    ///   Default: INI_DUPLICATE_MERGE_AND_OVERWRITE
    /// @param allowQuoted
    ///   Values on quotes are treat as a single value, otherwise the
    ///   value is retrieved just up to the next black char.
    ///   Default: true.
    /// @param allowBackslashes
    ///   Lines ending with a backslash (\) separator are joined with the
    ///   line bellow and treated as a single line.
    ///   Default: true.
    /// @param allowGlobals
    ///
    /// @param allowHierarchy
    ///
    /// @param hierarchyDelimiter
    ///
    /// @param keyValueDelimiter
    ///
    explicit Ini(
        const std::string &filename,
        uint8_t            commentType          = INI_COMMENT_DEFAULT,
        uint8_t            sectionDuplicateMode = INI_DUPLICATE_DISALLOW,
        uint8_t            valueDuplicateMode   = INI_DUPLICATE_DISALLOW,
        bool               allowQuoted          = true,
        bool               allowBackslashes     = true,
        bool               allowGlobals         = true,
        bool               allowHierarchy       = true,
        char               hierarchyDelimiter   = '/',
        char               keyValueDelimiter    = '=');

    explicit Ini(
        uint8_t commentType          = INI_COMMENT_DEFAULT,
        uint8_t sectionDuplicateMode = INI_DUPLICATE_DISALLOW,
        uint8_t valueDuplicateMode   = INI_DUPLICATE_DISALLOW,
        bool    allowQuoted          = true,
        bool    allowBackslashes     = true,
        bool    allowGlobals         = true,
        bool    allowHierarchy       = true,
        char    hierarchyDelimiter   = '/',
        char    keyValueDelimiter    = '=');

    //------------------------------------------------------------------------//
    //                                                                        //
    //------------------------------------------------------------------------//
public:
    void Save(const std::string &path);


    //------------------------------------------------------------------------//
    // Add Section                                                            //
    //------------------------------------------------------------------------//
public:
    void AddSection(const std::string &name);


    //------------------------------------------------------------------------//
    // Remove Section                                                         //
    //------------------------------------------------------------------------//
public:
    void RemoveSection(const std::string &name);


    //------------------------------------------------------------------------//
    // Get Section                                                            //
    //------------------------------------------------------------------------//
public:
    ///-------------------------------------------------------------------------
    /// @brief
    ///   Gets the section at path.
    /// @param path
    ///   The paths is fully qualified name of the section, separated by a /
    ///   (forward slash) regardless of the separator found on the INI file.
    /// @returns
    ///   A Section that matches the path.
    /// @throws
    ///   An std::invalid_argument if any Section can be found with the path.
    /// @see
    ///   SectionExists().
    const Section& GetSection(const std::string &path) const;

    ///-------------------------------------------------------------------------
    /// @brief
    ///   Retrieves all the sections found in INI file.
    /// @notes
    ///   If allowGlobals is set to true in the constructor and any values
    ///   are found outside any section, an Global section will be insert
    ///   automatically.
    /// @returns
    ///   A vector of Sections.
    const std::vector<Section>& GetSections() const noexcept;

    ///-------------------------------------------------------------------------
    /// @brief
    ///   Retrieves all the sections' names found in INI file.
    /// @notes
    ///   If allowGlobals is set to true in the constructor and any values
    ///   are found outside any section, an Global section will be insert
    ///   automatically. This section has the name of CoreIni::Global::Section.
    /// @returns
    ///   A vector of strings.
    std::vector<std::string> GetSectionNames() const noexcept;

    ///-------------------------------------------------------------------------
    /// @brief
    ///   Gets the section at path.
    /// @param path
    ///   The paths is fully qualified name of the section, separated by a /
    ///   (forward slash) regardless of the separator found on the INI file.
    /// @returns
    ///   true if a Section that matches the path is found, false otherwise.
    bool SectionExists(const std::string &path) const noexcept;


    //------------------------------------------------------------------------//
    // Add Value                                                              //
    //------------------------------------------------------------------------//
public:
    void AddValue(
        const std::string &sectionName,
        const std::string &valueName,
        const std::string &valueContent);


    //------------------------------------------------------------------------//
    // Remove Value                                                           //
    //------------------------------------------------------------------------//
public:
    void RemoveValue(
        const std::string &sectionName,
        const std::string &valueName);


    //------------------------------------------------------------------------//
    // Get Value                                                              //
    //------------------------------------------------------------------------//
public:
    const Value& GetValue(
        const std::string &sectionName,
        const std::string &valueName) const;


    bool ValueExists(
        const std::string &section,
        const std::string &name) const noexcept;


    template <typename T>
    const T GetValueAs(
        const std::string &sectionName,
        const std::string &valueName) const
    {
        auto value = GetValue(sectionName, valueName);

        std::stringstream ss;
        ss << value.m_content;

        T temp;
        ss >> temp;

        return temp;
    }


    //------------------------------------------------------------------------//
    // Private Methods                                                        //
    //------------------------------------------------------------------------//
private:
    void Parse(const std::vector<std::string> &lines);

    bool IsCommentLine(const std::string &line) const noexcept;

    bool IsSectionLine(
        const std::string &line,
        std::string       *pOut_SectionName) const noexcept;

    bool IsValueLine(
        const std::string        &line,
        std::vector<std::string> *pOut_KeyVal) const noexcept;


    std::string StripComments(const std::string &line) const noexcept;


    //------------------------------------------------------------------------//
    // iVars                                                                  //
    //------------------------------------------------------------------------//
private:
    std::vector<Section> m_sections;

    // Comment Type.
    uint8_t m_commentType;
    // Duplicate Mode.
    uint8_t m_sectionDuplicateMode;
    uint8_t m_valueDuplicateMode;
    // Housekeeping.
    bool     m_allowQuoted;
    bool     m_allowBackslashes;
    bool     m_allowGlobals;
    bool     m_allowHierarchy;
    char     m_hierarchyDelimiter;
    char     m_keyValueDelimiter;

}; // class Ini.

NS_COREINI_END
