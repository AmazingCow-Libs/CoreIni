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

class Ini
{
    //------------------------------------------------------------------------//
    // Inner Types                                                            //
    //------------------------------------------------------------------------//
    struct Value
    {
        std::string name;
        std::string value;
    };

    struct Section
    {
        const static char* KGlobalName;

        std::string        name;
        std::vector<Value> values;
    };


    //------------------------------------------------------------------------//
    // Enums / Constants / Typedefs                                           //
    //------------------------------------------------------------------------//
    enum {
        INI_COMMENT_SEMICOLON  = 1 << 0,
        INI_COMMENT_HASH       = 1 << 1,
        INT_COMMENT_ALL        = INI_COMMENT_HASH | INI_COMMENT_SEMICOLON,
        INI_COMMENT_DEFAULT    = INT_COMMENT_ALL,
        INI_COMMENT_NONE       = 0
    };

    //--------------------------------------------------------------------------
    // Notice about enums:
    //   The first enum must be a "naked" enum type because we want that
    //   bitwise logic easily implemented, but that enum is the only one
    //   that needs to be that way. ALl other enums could be better written
    //   as the new enum class types, but in sake of the consistency of
    //   the library, all the enums of this class will be "naked ones."
    //--------------------------------------------------------------------------
    enum {
        INI_DUPLICATE_DISALLOW,
        INI_DUPLICATE_OVERWRITE,
        INI_DUPLICATE_MERGE_AND_IGNORE,
        INI_DUPLICATE_MERGE_AND_OVERWRITE
    };


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
    /// @param hierachyDelimiter
    ///
    /// @param keyValueDelimiter
    ///
    explicit Ini(
        const std::string &filename,
        unsigned int       commentType        = INI_COMMENT_DEFAULT,
        unsigned int       duplicateMode      = INI_DUPLICATE_MERGE_AND_OVERWRITE,
        bool               allowQuoted        = true,
        bool               allowBackslashes   = true,
        bool               allowGlobals       = true,
        bool               allowHierarchy     = true,
        char               hierarchyDelimiter = '/',
        char               keyValueDelimiter  = '=');

    //------------------------------------------------------------------------//
    // Section                                                                //
    //------------------------------------------------------------------------//
public:
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
    ///   A Section that matches the path.
    /// @throws
    ///   An std::invalid_argument if any Section can be found with the path.
    /// @see
    ///   SectionExists().
    const Section& GetSection(const std::string &path) const;

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
    // Values                                                                 //
    //------------------------------------------------------------------------//
public:
    const Value& GetValue(const Section &section, const std::string &name);
    const Value& GetValue(const std::string &sectionName, const std::string &name);

    bool ValueExists(
        const Section     &section,
        const std::string &name) const noexcept;

    bool ValueExists(
        const std::string &section,
        const std::string &name) const noexcept;


    template <typename T>
    const T GetValueAs(const Section &section, const std::string &name)
    {
        auto value = GetValue(section, name);

        std::stringstream ss;
        ss << value.value;

        T temp;
        ss >> temp;

        return temp;
    }

    template <typename T>
    const T GetValueAs(const std::string &sectionName, const std::string &name)
    {
        return GetValueAs<T>(GetSection(sectionName), name);
    }

    // Private Methods
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

    // iVars
private:
    std::vector<Section> m_sections;

    unsigned int m_commentType;
    unsigned int m_duplicateMode;
    bool         m_allowQuoted;
    bool         m_allowBackslashes;
    bool         m_allowGlobals;
    bool         m_allowHierarchy;
    char         m_hierarchyDelimiter;
    char         m_keyValueDelimiter;
};

NS_COREINI_END
