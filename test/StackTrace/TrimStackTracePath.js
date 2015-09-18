function TrimStackTracePath(line) {
    return line && line.replace(/\(.+\\test.StackTrace./ig, "(");
}
