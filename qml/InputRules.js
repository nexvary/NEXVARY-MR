.pragma library

function digits(value) {
    return String(value).replace(/[٠-٩]/g, c => String(c.charCodeAt(0) - 0x660))
                        .replace(/[۰-۹]/g, c => String(c.charCodeAt(0) - 0x6f0));
}
function utcDate(year, month, day, hour, minute, second) {
    const date = new Date(0);
    date.setUTCFullYear(year, month - 1, day);
    date.setUTCHours(hour, minute, second, 0);
    return date;
}
function daysInMonth(year, month) {
    return utcDate(year, month + 1, 0, 0, 0, 0).getUTCDate();
}
function parseUtc(value) {
    const match = /^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z$/.exec(String(value));
    if (!match) return null;
    const parts = match.slice(1).map(Number);
    if (parts[0] < 1 || parts[1] < 1 || parts[1] > 12 || parts[2] < 1 ||
        parts[2] > daysInMonth(parts[0], parts[1]) || parts[3] > 23 || parts[4] > 59 || parts[5] > 59) return null;
    return {year:parts[0], month:parts[1], day:parts[2], hour:parts[3], minute:parts[4], second:parts[5]};
}
function valid(value, kind) {
    const text = String(value);
    if (!text.trim() || /[\u0000-\u0008\u000b\u000c\u000e-\u001f\u007f]/.test(text)) return false;
    switch (kind) {
    case "phone": return /^\+[1-9][0-9]{6,14}$/.test(digits(text));
    case "email": return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(text);
    case "integer": return /^[+-]?\d+$/.test(digits(text));
    case "number": return /^[+-]?(?:\d+(?:\.\d+)?|\.\d+)$/.test(digits(text));
    case "dateTime": return parseUtc(text) !== null;
    case "username": return /^[a-z0-9][a-z0-9._-]{2,63}$/.test(text);
    case "password":
        try { return text.length >= 14 && unescape(encodeURIComponent(text)).length <= 256; }
        catch (error) { return false; }
    case "apiKey": return text.length <= 4096 && /^[A-Za-z0-9._~+\/-]+=*$/.test(text);
    case "uuid": return /^[0-9a-f]{8}(?:-[0-9a-f]{4}){3}-[0-9a-f]{12}$/i.test(text) && text !== "00000000-0000-0000-0000-000000000000";
    default: return true;
    }
}
function state(value, required, kind) {
    return !String(value).trim() ? (required ? "empty" : "neutral") : valid(value, kind) ? "valid" : "invalid";
}
