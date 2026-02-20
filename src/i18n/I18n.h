#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace ComfyX {

class I18n {
public:
    static I18n& instance();

    void initialize(const std::string& language = "en");
    void setLanguage(const std::string& language);

    const std::string& translate(const std::string& key) const;
    const std::string& currentLanguage() const { return m_language; }

    // Convenience static method
    static const std::string& t(const std::string& key) {
        return instance().translate(key);
    }

private:
    I18n() = default;

    void loadTranslations();
    void loadDefaults();

    std::string m_language = "en";
    std::unordered_map<std::string, std::string> m_translations;
    static const std::string s_missing;
};

} // namespace ComfyX
