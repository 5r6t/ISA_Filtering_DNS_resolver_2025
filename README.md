# ISA_Filtering_DNS_resolver_2025
# ISA Project – Concise Assignment Summary

## General Requirements
- Implement a communicating application using **BSD sockets** (unless specified otherwise).
- Language: **C/C++**.
- Must compile & run on **merlin.fit.vutbr.cz (GNU/Linux)**.
- Code must be **portable** (may be tested on different Linux distros/architectures).
- If special library versions are required (available on merlin), note in **documentation & README**.

## Submission Rules
- Submit `.tar` archive named **xlogin00.tar** via IS VUT (not compressed further).
- Deadline: **17.11.2025 (hard)**.  
  – No late submission, email submission, or fixes.
- Archive must include:
  - **Source code** (with author & login in header, file names per assignment).
  - **Makefile** (must build source).
  - **Documentation (manual.pdf)**:
    - intro, design, implementation, usage, testing (methods & results),
    - structured like a technical report (title page, ToC, numbered chapters, references, etc.).
  - **README**:
    - author name & login, date, program description (extensions/limits),
    - run example, list of submitted files.
  - Other required files depending on assignment.
- Missing features → clearly documented in **manual.pdf** & **README**.
- Follow Unix conventions, structured modular code, comments, formatting.
- Program must:
  - include help/usage info,
  - handle invalid input with error/help,
  - never crash (no segfaults, div by zero, etc.).
- Thorough testing required; include results in documentation.
- External code (tutorials/examples) → mark clearly, cite source & license.
- **Generated code (e.g. ChatGPT)** = plagiarism.
- Follow Moodle forum for clarifications.
- Before submission: verify names, files, Makefile build on target platform.

## Grading
- **Max: 20 points**
  - 15 – functional application
  - 5 – documentation (only if code works)
- Deductions:
  - messy/uncommented code: -7
  - missing/faulty Makefile: -4
  - poor/missing documentation: -5
  - wrong input/output/config format: -10
  - cannot compile/run/test: 0
  - late submission: 0
  - not following assignment: 0
  - non-functional code: 0
  - plagiarism/shared code: 0 + disciplinary action

## Testing & Validation Checklist
- Enable compiler warnings (`-Wall`).
- Check memory use & initialization (e.g. **valgrind**).
- Debug program execution.
- Use tools (Wireshark, dig, openssl, etc.) where relevant.
- Create **repeatable automated tests** (unit, integration, system).
- Common regex mistakes:
  - unnecessary use,
  - incorrect syntax (e.g. limiting URLs wrongly).
- Verify:
  - file naming & type (.tar unpackable with `tar`),
  - buildable with `make`,
  - correct output file in project root,
  - no runtime crashes,
  - inclusion of **README & documentation**,
  - proper project root structure,
  - meaningful tests,
  - documentation shows understanding of theory, design, testing, and deviations.

# ISA Project Variant – Filtering DNS Resolver

## Task
- Write program **dns**:
  - Filter **A-type queries** targeting domains from a given blocklist (and their subdomains).
  - Forward all other queries unchanged to a specified resolver.
  - Forward responses back to the original client.
  - Must implement DNS message **parsing/building directly in program**.
  - Only handle **UDP** and **A queries**.
  - Other query types / blocked queries → reply with appropriate **error message**.

## Allowed Libraries
- Sockets, networking (`netinet/*`, `sys/*`, `arpa/*`).
- Threads (`pthread`), signals, time.
- Standard **C (ISO/ANSI, POSIX)** and **C++ STL**.
- **No other libraries allowed.**

## Program Execution
- **Usage:**  
  `dns -s server [-p port] -f filter_file`
- Parameters (any order):
  - `-s`: DNS resolver address (IP/domain).
  - `-p port`: listening port (default: 53).
  - `-f filter_file`: file with blocked domains.

## Supported Queries
- Only **A-type queries**.
- Transport: **UDP**.
- No **DNSSEC** support required.

## Output
- By default: **no output**.
- Optional: `-v` → verbose logging in custom format.

## Filter File Format
- Text (ASCII), one domain per line.
- Ignore:
  - empty lines,
  - lines starting with `#`.
- Must handle line endings from Linux, Windows, macOS.
- Example list:  
  [Adservers list](https://pgl.yoyo.org/adservers/serverlist.php?hostformat=nohtml&showintro=1)

## Additional Notes
- Read common ISA project rules before submission.
- Document all extensions in **README & manual.pdf** (no extra points).
- Program must handle **invalid input gracefully**.
- Errors → print to **stderr** clearly.
- If spec missing details → document chosen solution & rationale.
- Document:
  - method for traversing blocked domain list,
  - generated error messages and conditions.
- Code: **modular, tested, robust**.
  - Tests runnable via `make test`.
  - Follow **robustness principle** (tolerant to small input deviations).
- Apply previous course practices:
  - code formatting, comments, file naming, user-friendliness.

## Reference Environment
- Program must compile & run on:
  - **eva.fit.vutbr.cz**
  - **merlin.fit.vutbr.cz**
- Should be portable (Linux, FreeBSD, other architectures/distros/library versions).
- If requiring minimum library version (available on eva/merlin), state in **documentation & README**.

## Recommended Reading
- [RFC 1035 – Domain Names](https://datatracker.ietf.org/doc/html/rfc1035)


# GOOD TO KNOW
> ad více dotazů v jedné zprávě DNS: takové zprávy se v praxi nevyskytují, protože není jasná sémantika některých polí v očekávané odpovědi (např. AA, RCODE), vizte atké např. https://stackoverflow.com/questions/4082081/requesting-a-and-aaaa-records-in-single-dns-query/4083071#4083071. Zasílat více dotazů v jedné zprávě při testování nebudu, nicméně v dokumentaci doporučuji vysvětlit, jak se program v takové situaci zachová.
> Pouzivame UDP -- max velkost packetu 512bytes a iba jeden
> DNS header = 12 bytes



> Ad komprese jmen: Samozřejmě musíte podporovat, jinak vám program v praxi fungovat nebude správně. Komprese jmen je naprosto běžně používaná technika.

# SOURCES
- dual stack https://stackoverflow.com/questions/22075363/dual-stack-with-one-socket
- ipv6, sockaddr_storage https://datatracker.ietf.org/doc/html/rfc2553
- DNS https://mislove.org/teaching/cs4700/spring11/handouts/project1-primer.pdf
- DNS https://learn.microsoft.com/en-us/windows-server/networking/dns/message-formats
