#include "pch.h"
#include "web/chtml.h"
using namespace CTML;

z_string testChtml() {
    // 1. Initialize the document object
    Document doc;

    // 2. Configure the document head
    doc.AppendNodeToHead(Node("title", "CTML Page Example"));
    doc.AppendNodeToHead(Node("link").SetAttribute("rel", "stylesheet").SetAttribute("href", "style.css"));

    // 3. Create structural body elements
    Node header_div("div.container#main-header"); // Shorthand for class and ID
    header_div.AppendChild(Node("h1", "Welcome to CTML"));

    Node content_div("div.content-body");
    
    // Create a paragraph and chain class and inline style attributes
    Node paragraph("p", "This page was fully generated programmatically using C++.");
    paragraph.SetAttribute("class", "lead-text").SetAttribute("style", "color: #333;");
    content_div.AppendChild(paragraph);

    // Create an anchor link with an external target
    Node link("a", "Visit CTML GitHub");
    link.SetAttribute("href", "https://github.com").SetAttribute("target", "_blank");
    content_div.AppendChild(link);

    // 4. Assemble the final document structure
    doc.AppendNodeToBody(header_div);
    doc.AppendNodeToBody(content_div);

    // 5. Output the rendered HTML string (true enables human-readable spacing)
    //std::string html_string = doc.ToString(true);
    //std::cout << html_string << std::endl;

    return doc.ToString();;
}
