#include "Drift/UI/FontSystem/FontMetrics.h"
#include "Drift/UI/FontSystem/Font.h"
#include "Drift/Core/Log.h"
#include "Drift/Core/Profiler.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>

namespace Drift::UI {

namespace TextUtils {

std::string RemoveAccents(const std::string& text) {
    DRIFT_PROFILE_FUNCTION();
    
    std::string result;
    result.reserve(text.length());
    
    for (char c : text) {
        switch (c) {
            // Vogais com acento agudo
            case 'á': case 'à': case 'ã': case 'â': case 'ä': result += 'a'; break;
            case 'é': case 'è': case 'ê': case 'ë': result += 'e'; break;
            case 'í': case 'ì': case 'î': case 'ï': result += 'i'; break;
            case 'ó': case 'ò': case 'õ': case 'ô': case 'ö': result += 'o'; break;
            case 'ú': case 'ù': case 'û': case 'ü': result += 'u'; break;
            case 'ý': case 'ÿ': result += 'y'; break;
            
            // Vogais maiúsculas com acento
            case 'Á': case 'À': case 'Ã': case 'Â': case 'Ä': result += 'A'; break;
            case 'É': case 'È': case 'Ê': case 'Ë': result += 'E'; break;
            case 'Í': case 'Ì': case 'Î': case 'Ï': result += 'I'; break;
            case 'Ó': case 'Ò': case 'Õ': case 'Ô': case 'Ö': result += 'O'; break;
            case 'Ú': case 'Ù': case 'Û': case 'Ü': result += 'U'; break;
            case 'Ý': result += 'Y'; break;
            
            // Consoantes especiais
            case 'ç': result += 'c'; break;
            case 'Ç': result += 'C'; break;
            case 'ñ': result += 'n'; break;
            case 'Ñ': result += 'N'; break;
            
            default: result += c; break;
        }
    }
    
    return result;
}

std::string ToLower(const std::string& text) {
    DRIFT_PROFILE_FUNCTION();
    
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string ToUpper(const std::string& text) {
    DRIFT_PROFILE_FUNCTION();
    
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string ToTitleCase(const std::string& text) {
    DRIFT_PROFILE_FUNCTION();
    
    if (text.empty()) {
        return text;
    }
    
    std::string result = ToLower(text);
    bool capitalize = true;
    
    for (char& c : result) {
        if (std::isspace(c) || c == '-' || c == '_') {
            capitalize = true;
        } else if (capitalize) {
            c = std::toupper(c);
            capitalize = false;
        }
    }
    
    return result;
}

std::string TruncateWithEllipsis(const std::string& text, 
                                 const std::shared_ptr<Font>& font,
                                 float maxWidth) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!font) {
        return text;
    }
    
    // Medir texto completo
    float textWidth = 0.0f;
    for (char c : text) {
        const GlyphInfo* glyph = font->GetGlyph(static_cast<uint32_t>(c));
        if (glyph) {
            textWidth += glyph->advance;
        }
    }
    
    if (textWidth <= maxWidth) {
        return text;
    }
    
    // Medir largura do "..." (aproximadamente 3 caracteres)
    float ellipsisWidth = 0.0f;
    for (int i = 0; i < 3; ++i) {
        const GlyphInfo* glyph = font->GetGlyph(static_cast<uint32_t>('.'));
        if (glyph) {
            ellipsisWidth += glyph->advance;
        }
    }
    
    // Calcular quantos caracteres cabem
    float availableWidth = maxWidth - ellipsisWidth;
    if (availableWidth <= 0) {
        return "...";
    }
    
    std::string result;
    float currentWidth = 0.0f;
    
    for (char c : text) {
        const GlyphInfo* glyph = font->GetGlyph(static_cast<uint32_t>(c));
        if (glyph) {
            if (currentWidth + glyph->advance > availableWidth) {
                break;
            }
            result += c;
            currentWidth += glyph->advance;
        }
    }
    
    return result + "...";
}

std::string FormatNumber(int64_t number, const std::string& separator) {
    DRIFT_PROFILE_FUNCTION();
    
    std::stringstream ss;
    ss << std::abs(number);
    std::string numStr = ss.str();
    
    // Adicionar separadores
    for (int i = numStr.length() - 3; i > 0; i -= 3) {
        numStr.insert(i, separator);
    }
    
    return (number < 0 ? "-" : "") + numStr;
}

std::string FormatDecimal(double number, int precision) {
    DRIFT_PROFILE_FUNCTION();
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << number;
    return ss.str();
}

std::string FormatTime(float seconds) {
    DRIFT_PROFILE_FUNCTION();
    
    if (seconds < 0) {
        return "0:00";
    }
    
    int totalSeconds = static_cast<int>(seconds);
    int minutes = totalSeconds / 60;
    int remainingSeconds = totalSeconds % 60;
    
    if (minutes > 0) {
        return std::to_string(minutes) + ":" + 
               (remainingSeconds < 10 ? "0" : "") + std::to_string(remainingSeconds);
    } else {
        return "0:" + (remainingSeconds < 10 ? "0" : "") + std::to_string(remainingSeconds);
    }
}

std::string FormatBytes(size_t bytes) {
    DRIFT_PROFILE_FUNCTION();
    
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::stringstream ss;
    if (unitIndex == 0) {
        ss << static_cast<int>(size) << " " << units[unitIndex];
    } else {
        ss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
    }
    
    return ss.str();
}

} // namespace TextUtils

// Funções auxiliares para layout de texto
namespace {

std::vector<uint32_t> DecodeUTF8(const std::string& utf8_string) {
    DRIFT_PROFILE_FUNCTION();
    
    std::vector<uint32_t> codepoints;
    codepoints.reserve(utf8_string.length());
    
    for (size_t i = 0; i < utf8_string.length();) {
        uint32_t codepoint = 0;
        unsigned char byte = static_cast<unsigned char>(utf8_string[i]);
        
        if (byte < 0x80) {
            // ASCII
            codepoint = byte;
            i += 1;
        } else if ((byte & 0xE0) == 0xC0) {
            // 2 bytes
            if (i + 1 < utf8_string.length()) {
                codepoint = ((byte & 0x1F) << 6) | (static_cast<unsigned char>(utf8_string[i + 1]) & 0x3F);
                i += 2;
            } else {
                i += 1; // Incomplete sequence
            }
        } else if ((byte & 0xF0) == 0xE0) {
            // 3 bytes
            if (i + 2 < utf8_string.length()) {
                codepoint = ((byte & 0x0F) << 12) | 
                           ((static_cast<unsigned char>(utf8_string[i + 1]) & 0x3F) << 6) |
                           (static_cast<unsigned char>(utf8_string[i + 2]) & 0x3F);
                i += 3;
            } else {
                i += 1; // Incomplete sequence
            }
        } else if ((byte & 0xF8) == 0xF0) {
            // 4 bytes
            if (i + 3 < utf8_string.length()) {
                codepoint = ((byte & 0x07) << 18) |
                           ((static_cast<unsigned char>(utf8_string[i + 1]) & 0x3F) << 12) |
                           ((static_cast<unsigned char>(utf8_string[i + 2]) & 0x3F) << 6) |
                           (static_cast<unsigned char>(utf8_string[i + 3]) & 0x3F);
                i += 4;
            } else {
                i += 1; // Incomplete sequence
            }
        } else {
            // Invalid byte
            i += 1;
        }
        
        if (codepoint > 0) {
            codepoints.push_back(codepoint);
        }
    }
    
    return codepoints;
}

bool IsWhitespace(uint32_t codepoint) {
    return codepoint == 0x0020 || // Space
           codepoint == 0x0009 || // Tab
           codepoint == 0x000A || // Line feed
           codepoint == 0x000D || // Carriage return
           codepoint == 0x00A0 || // Non-breaking space
           codepoint == 0x2000 || // En quad
           codepoint == 0x2001 || // Em quad
           codepoint == 0x2002 || // En space
           codepoint == 0x2003 || // Em space
           codepoint == 0x2004 || // Three-per-em space
           codepoint == 0x2005 || // Four-per-em space
           codepoint == 0x2006 || // Six-per-em space
           codepoint == 0x2007 || // Figure space
           codepoint == 0x2008 || // Punctuation space
           codepoint == 0x2009 || // Thin space
           codepoint == 0x200A || // Hair space
           codepoint == 0x202F || // Narrow no-break space
           codepoint == 0x205F || // Medium mathematical space
           codepoint == 0x3000;   // Ideographic space
}

bool IsLineBreak(uint32_t codepoint) {
    return codepoint == 0x000A || // Line feed
           codepoint == 0x000D || // Carriage return
           codepoint == 0x000B || // Vertical tab
           codepoint == 0x000C || // Form feed
           codepoint == 0x0085 || // Next line
           codepoint == 0x2028 || // Line separator
           codepoint == 0x2029;   // Paragraph separator
}

std::vector<std::string> SplitIntoWords(const std::string& text) {
    DRIFT_PROFILE_FUNCTION();
    
    std::vector<std::string> words;
    std::string currentWord;
    
    for (char c : text) {
        if (std::isspace(c)) {
            if (!currentWord.empty()) {
                words.push_back(currentWord);
                currentWord.clear();
            }
        } else {
            currentWord += c;
        }
    }
    
    if (!currentWord.empty()) {
        words.push_back(currentWord);
    }
    
    return words;
}

} // namespace

// Implementação das funções de layout de texto
TextLayoutResult CalculateTextLayout(const std::string& text, 
                                    const std::shared_ptr<Font>& font,
                                    const TextLayoutConfig& layoutConfig) {
    DRIFT_PROFILE_FUNCTION();
    
    TextLayoutResult result;
    
    if (!font || text.empty()) {
        return result;
    }
    
    // Decodificar UTF-8
    auto codepoints = DecodeUTF8(text);
    if (codepoints.empty()) {
        return result;
    }
    
    // Configurar layout
    float currentX = 0.0f;
    float currentY = 0.0f;
    float lineHeight = font->GetMetrics().lineHeight * layoutConfig.lineSpacing;
    float maxLineWidth = 0.0f;
    
    std::vector<TextLineInfo> lines;
    std::vector<TextCharInfo> chars;
    
    TextLineInfo currentLine;
    currentLine.startIndex = 0;
    currentLine.baseline = currentY + font->GetMetrics().ascent;
    
    size_t charIndex = 0;
    size_t lineIndex = 0;
    
    for (size_t i = 0; i < codepoints.size(); ++i) {
        uint32_t codepoint = codepoints[i];
        
        // Verificar quebra de linha
        if (IsLineBreak(codepoint)) {
            // Finalizar linha atual
            currentLine.endIndex = charIndex;
            currentLine.text = text.substr(currentLine.startIndex, currentLine.endIndex - currentLine.startIndex);
            currentLine.size = glm::vec2(currentX, lineHeight);
            currentLine.position = glm::vec2(0.0f, currentY);
            currentLine.isLastLine = false;
            lines.push_back(currentLine);
            
            // Iniciar nova linha
            currentY += lineHeight;
            currentX = 0.0f;
            lineIndex++;
            
            currentLine.startIndex = charIndex + 1;
            currentLine.baseline = currentY + font->GetMetrics().ascent;
            continue;
        }
        
        // Obter glyph
        const GlyphInfo* glyph = font->GetGlyph(codepoint);
        if (!glyph) {
            // Usar fallback
            auto fallbackFont = font->GetFallbackFont();
            if (fallbackFont) {
                glyph = fallbackFont->GetGlyph(codepoint);
            }
            
            if (!glyph) {
                // Usar glyph de substituição
                glyph = font->GetGlyph(0x003F); // Question mark
                if (!glyph) {
                    continue; // Pular caractere
                }
            }
        }
        
        // Verificar quebra de linha por largura
        if (layoutConfig.maxWidth > 0.0f && 
            currentX + glyph->advance > layoutConfig.maxWidth &&
            layoutConfig.enableWordWrap) {
            
            // Finalizar linha atual
            currentLine.endIndex = charIndex;
            currentLine.text = text.substr(currentLine.startIndex, currentLine.endIndex - currentLine.startIndex);
            currentLine.size = glm::vec2(currentX, lineHeight);
            currentLine.position = glm::vec2(0.0f, currentY);
            currentLine.isLastLine = false;
            lines.push_back(currentLine);
            
            // Iniciar nova linha
            currentY += lineHeight;
            currentX = 0.0f;
            lineIndex++;
            
            currentLine.startIndex = charIndex;
            currentLine.baseline = currentY + font->GetMetrics().ascent;
        }
        
        // Adicionar caractere
        TextCharInfo charInfo;
        charInfo.codepoint = codepoint;
        charInfo.position = glm::vec2(currentX, currentY);
        charInfo.size = glyph->size;
        charInfo.uv0 = glyph->uv0;
        charInfo.uv1 = glyph->uv1;
        charInfo.advance = glyph->advance;
        charInfo.isWhitespace = IsWhitespace(codepoint);
        charInfo.isLineBreak = IsLineBreak(codepoint);
        charInfo.lineIndex = lineIndex;
        charInfo.charIndex = chars.size();
        chars.push_back(charInfo);
        
        // Atualizar posição
        currentX += glyph->advance + layoutConfig.letterSpacing;
        
        // Adicionar kerning se habilitado
        if (layoutConfig.enableKerning && i + 1 < codepoints.size()) {
            currentX += font->GetKerning(codepoint, codepoints[i + 1]);
        }
        
        maxLineWidth = std::max(maxLineWidth, currentX);
        charIndex++;
    }
    
    // Finalizar última linha
    if (!chars.empty()) {
        currentLine.endIndex = charIndex;
        currentLine.text = text.substr(currentLine.startIndex, currentLine.endIndex - currentLine.startIndex);
        currentLine.size = glm::vec2(currentX, lineHeight);
        currentLine.position = glm::vec2(0.0f, currentY);
        currentLine.isLastLine = true;
        lines.push_back(currentLine);
    }
    
    // Aplicar alinhamento horizontal
    for (auto& line : lines) {
        float lineWidth = line.size.x;
        switch (layoutConfig.horizontalAlign) {
            case TextAlign::Center:
                line.position.x = (layoutConfig.maxWidth - lineWidth) * 0.5f;
                break;
            case TextAlign::Right:
                line.position.x = layoutConfig.maxWidth - lineWidth;
                break;
            case TextAlign::Justify:
                // TODO: Implementar justificação
                break;
            default:
                line.position.x = 0.0f;
                break;
        }
    }
    
    // Aplicar alinhamento vertical
    float totalHeight = currentY + lineHeight;
    float startY = 0.0f;
    
    switch (layoutConfig.verticalAlign) {
        case TextVerticalAlign::Middle:
            startY = (layoutConfig.maxWidth > 0.0f ? layoutConfig.maxWidth : totalHeight) - totalHeight;
            startY *= 0.5f;
            break;
        case TextVerticalAlign::Bottom:
            startY = (layoutConfig.maxWidth > 0.0f ? layoutConfig.maxWidth : totalHeight) - totalHeight;
            break;
        case TextVerticalAlign::Baseline:
            startY = 0.0f;
            break;
        default:
            startY = 0.0f;
            break;
    }
    
    // Ajustar posições Y
    for (auto& line : lines) {
        line.position.y += startY;
    }
    
    for (auto& charInfo : chars) {
        charInfo.position.y += startY;
    }
    
    // Preencher resultado
    result.lines = lines;
    result.chars = chars;
    result.totalSize = glm::vec2(maxLineWidth, totalHeight);
    result.lineCount = lines.size();
    result.charCount = chars.size();
    result.maxLineWidth = maxLineWidth;
    result.totalHeight = totalHeight;
    result.wasTruncated = false; // TODO: Implementar truncamento
    
    return result;
}

float CalculateTextWidth(const std::string& text, const std::shared_ptr<Font>& font) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!font || text.empty()) {
        return 0.0f;
    }
    
    float width = 0.0f;
    auto codepoints = DecodeUTF8(text);
    
    for (size_t i = 0; i < codepoints.size(); ++i) {
        const GlyphInfo* glyph = font->GetGlyph(codepoints[i]);
        if (glyph) {
            width += glyph->advance;
            
            // Adicionar kerning
            if (i + 1 < codepoints.size()) {
                width += font->GetKerning(codepoints[i], codepoints[i + 1]);
            }
        }
    }
    
    return width;
}

float CalculateTextHeight(const std::string& text, const std::shared_ptr<Font>& font,
                         const TextLayoutConfig& layoutConfig) {
    DRIFT_PROFILE_FUNCTION();
    
    if (!font || text.empty()) {
        return 0.0f;
    }
    
    auto layout = CalculateTextLayout(text, font, layoutConfig);
    return layout.totalHeight;
}

} // namespace Drift::UI 