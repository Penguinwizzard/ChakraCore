function TrimStackTracePath(line) {
    return line && line.replace(/\(.+\\test.[^\\/]*./ig, "(");
}
