%{
/*
 * Copyright 2013 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * Author: Author: ncardwell@google.com (Neal Cardwell)
 *
 * This is the parser for the packetdrill script language. It is
 * processed by the bison parser generator.
 *
 * For full documentation see: http://www.gnu.org/software/bison/manual/
 *
 * Here is a quick and dirty tutorial on bison:
 *
 * A bison parser specification is basically a BNF grammar for the
 * language you are parsing. Each rule specifies a nonterminal symbol
 * on the left-hand side and a sequence of terminal symbols (lexical
 * tokens) and or nonterminal symbols on the right-hand side that can
 * "reduce" to the symbol on the left hand side. When the parser sees
 * the sequence of symbols on the right where it "wants" to see a
 * nonterminal on the left, the rule fires, executing the semantic
 * action code in curly {} braces as it reduces the right hand side to
 * the left hand side.
 *
 * The semantic action code for a rule produces an output, which it
 * can reference using the $$ token. The set of possible types
 * returned in output expressions is given in the %union section of
 * the .y file. The specific type of the output for a terminal or
 * nonterminal symbol (corresponding to a field in the %union) is
 * given by the %type directive in the .y file. The action code can
 * access the outputs of the symbols on the right hand side by using
 * the notation $1 for the first symbol, $2 for the second symbol, and
 * so on.
 *
 * The lexer (generated by flex from lexer.l) feeds a stream of
 * terminal symbols up to this parser. Parser semantic actions can
 * access the lexer output for a terminal symbol with the same
 * notation they use for nonterminals.
 *
 */

/* The first part of the .y file consists of C code that bison copies
 * directly into the top of the .c file it generates.
 */


#include "inet/common/INETDefs.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fmemopen.h"

#include "PacketDrillUtils.h"
#include "PacketDrill.h"


/* This include of the bison-generated .h file must go last so that we
 * can first include all of the declarations on which it depends.
 */
#include "parser.h"

/* Change this YYDEBUG to 1 to get verbose debug output for parsing: */
#define YYDEBUG 0
#if YYDEBUG
extern int yydebug;
#endif

extern FILE *yyin;
extern int yylineno;
extern int yywrap(void);
extern char *yytext;
extern int yylex(void);
extern int yyparse(void);

/* The input to the parser: the path name of the script file to parse. */
static const char* current_script_path = NULL;

/* The starting line number of the input script statement that we're
 * currently parsing. This may be different than yylineno if bison had
 * to look ahead and lexically scan a token on the following line to
 * decide that the current statement is done.
 */
static int current_script_line = -1;

/*
 * We use this object to look up configuration info needed during
 * parsing.
 */
static PacketDrillConfig *in_config = NULL;

/* The output of the parser: an output script containing
 * 1) a linked list of options
 * 2) a linked list of events
 */
static PacketDrillScript *out_script = NULL;


/* The test invocation to pass back to parse_and_finalize_config(). */
struct invocation *invocation;

/* This standard callback is invoked by flex when it encounters
 * the end of a file. We return 1 to tell flex to return EOF.
 */
int yywrap(void)
{
    return 1;
}


/* The public entry point for the script parser. Parses the
 * text script file with the given path name and fills in the script
 * object with the parsed representation.
 */
int parse_script(PacketDrillConfig *config, PacketDrillScript *script, struct invocation *callback_invocation){
    /* This bison-generated parser is not multi-thread safe, so we
     * have a lock to prevent more than one thread using the
     * parser at the same time. This is useful in the wire server
     * context, where in general we may have more than one test
     * thread running at the same time.
     */

#if YYDEBUG
    yydebug = 1;
#endif

    /* Now parse the script from our buffer. */
    yyin = fmemopen(script->getBuffer(), script->getLength(), "r");
    if (yyin == NULL)
        printf("fmemopen: parse error opening script buffer");

    current_script_path = config->getScriptPath();
    in_config = config;
    printf("set out_script\n");
    out_script = script;
    invocation = callback_invocation;

    /* We have to reset the line number here since the wire server
     * can do more than one yyparse().
     */
    yylineno = 1;
    int result = yyparse(); /* invoke bison-generated parser */
    current_script_path = NULL;
    if (fclose(yyin))
        printf("fclose: error closing script buffer");

    /* Unlock parser. */

    return result ? -1 : 0;
}

/* Bison emits code to call this method when there's a parse-time error.
 * We print the line number and the error message.
 */
static void yyerror(const char *message) {
    fprintf(stderr, "%s:%d: parse error at '%s': %s\n",
        current_script_path, yylineno, yytext, message);
}


/* Create and initalize a new integer expression with the given
 * literal value and format string.
 */
static PacketDrillExpression *new_integer_expression(int64 num, const char *format) {
    PacketDrillExpression *expression = new PacketDrillExpression(EXPR_INTEGER);
    expression->setNum(num);
    expression->setFormat(format);
    return expression;
}


/* Create and initialize a new option. */
/*static struct option_list *new_option(char *name, char *value)
{
    return NULL;
}*/

%}

%locations
%expect 1  /* we expect a shift/reduce conflict for the | binary expression */
/* The %union section specifies the set of possible types for values
 * for all nonterminal and terminal symbols in the grammar.
 */
%union {
    int64 integer;
    double floating;
    char *string;
    char *reserved;
    int64 time_usecs;
    enum direction_t direction;
    uint16 port;
    int32 window;
    uint32 sequence_number;
    struct option_list *option;
    PacketDrillEvent *event;
    PacketDrillPacket *packet;
    struct syscall_spec *syscall;
    PacketDrillStruct *sack_block;
    PacketDrillExpression *expression;
    cQueue *expression_list;
    struct errno_spec *errno_info;
}

/* The specific type of the output for a symbol is given by the %type
 * directive. By convention terminal symbols returned from the lexer
 * have ALL_CAPS names, and nonterminal symbols have lower_case names.
 */
%token ELLIPSIS
%token <reserved> UDP
%token <reserved> OPTION
%token <floating> FLOAT
%token <integer> INTEGER HEX_INTEGER
%token <string> WORD MYSTRING
%type <direction> direction
%type <event> event events event_time action
%type <time_usecs> time opt_end_time
%type <packet> packet_spec udp_packet_spec
%type <packet> packet_prefix
%type <syscall> syscall_spec
%type <string> opt_note note word_list
%type <string> script
%type <string> function_name
%type <expression_list> expression_list function_arguments
%type <expression> expression binary_expression array
%type <expression> decimal_integer hex_integer
%type <errno_info> opt_errno

%%  /* The grammar follows. */

script
: events {
    $$ = NULL;    /* The parser output is in out_script */
}
;


events
: event {
    out_script->addEvent($1);    /* save pointer to event list as output of parser */
    $$ = $1;    /* return the tail so that we can append to it */
}
| events event {
    out_script->addEvent($2);
    $$ = $2;    /* return the tail so that we can append to it */
}
;

event
: event_time action {
    $$ = $2;
    $$->setLineNumber($1->getLineNumber());    /* use timestamp's line */
    $$->setEventTime($1->getEventTime());
    $$->setEventTimeEnd($1->getEventTimeEnd());
    $$->setTimeType($1->getTimeType());
    $1->getLineNumber(),
    $1->getEventTime().dbl(),
    $1->getEventTimeEnd().dbl(),
    $1->getTimeType();
    if ($$->getEventTimeEnd() != NO_TIME_RANGE) {
        if ($$->getEventTimeEnd() < $$->getEventTime())
            printf("time range is backwards");
    }
    if ($$->getTimeType() == ANY_TIME &&  ($$->getType() != PACKET_EVENT ||
        ($$->getPacket())->getDirection() != DIRECTION_OUTBOUND)) {
        yylineno = $$->getLineNumber();
        printf("event time <star> can only be used with outbound packets");
    } else if (($$->getTimeType() == ABSOLUTE_RANGE_TIME ||
        $$->getTimeType() == RELATIVE_RANGE_TIME) &&
        ($$->getType() != PACKET_EVENT ||
        ($$->getPacket())->getDirection() != DIRECTION_OUTBOUND)) {
        yylineno = $$->getLineNumber();
        printf("event time range can only be used with outbound packets");
    }
    free($1);
}
;

event_time
: '+' time {
    $$ = new PacketDrillEvent(INVALID_EVENT);
    $$->setLineNumber(@2.first_line);
    $$->setEventTime($2);
    $$->setTimeType(RELATIVE_TIME);
    printf("relative done\n");
}
| time {
    $$ = new PacketDrillEvent(INVALID_EVENT);
    $$->setLineNumber(@1.first_line);
    $$->setEventTime($1);
    $$->setTimeType(ABSOLUTE_TIME);
    printf("absolute done\n");
}
| '*' {
    $$ = new PacketDrillEvent(INVALID_EVENT);
    $$->setLineNumber(@1.first_line);
    $$->setTimeType(ANY_TIME);
    printf("any done\n");
}
| time '~' time {
    $$ = new PacketDrillEvent(INVALID_EVENT);
    $$->setLineNumber(@1.first_line);
    $$->setTimeType(ABSOLUTE_RANGE_TIME);
    $$->setEventTime($1);
    $$->setEventTimeEnd($3);
}
| '+' time '~' '+' time {
    $$ = new PacketDrillEvent(INVALID_EVENT);
    $$->setLineNumber(@1.first_line);
    $$->setTimeType(RELATIVE_RANGE_TIME);
    $$->setEventTime($2);
    $$->setEventTimeEnd($5);
}
;

time
: FLOAT {
    if ($1 < 0) {
        printf("negative time");
    }
    $$ = (int64)($1 * 1.0e6); /* convert float secs to s64 microseconds */
}
| INTEGER {
    if ($1 < 0) {
        printf("negative time");
    }
    $$ = (int64)($1 * 1000000); /* convert int secs to s64 microseconds */
}
;

action
: packet_spec {
    $$ = new PacketDrillEvent(PACKET_EVENT);  $$->setPacket($1);
}
| syscall_spec {
    $$ = new PacketDrillEvent(SYSCALL_EVENT);
    $$->setSyscall($1);
}
;

packet_spec
: udp_packet_spec {
    $$ = $1;
}
;


udp_packet_spec
: packet_prefix UDP '(' INTEGER ')' {
    char *error = NULL;
    PacketDrillPacket *outer = $1, *inner = NULL;

    enum direction_t direction = outer->getDirection();

    cPacket* pkt = PacketDrill::buildUDPPacket(in_config->getWireProtocol(), direction, $4, &error);
    if (direction == DIRECTION_INBOUND)
        pkt->setName("parserInbound");
    else
        pkt->setName("parserOutbound");
    inner = new PacketDrillPacket();
    inner->setInetPacket(pkt);
    inner->setDirection(direction);

    $$ = inner;
}
;


packet_prefix
: direction {
    $$ = new PacketDrillPacket();
    $$->setDirection($1);
}
;


direction
: '<' {
    $$ = DIRECTION_INBOUND;
    current_script_line = yylineno;
}
| '>' {
    $$ = DIRECTION_OUTBOUND;
    current_script_line = yylineno;
}
;


syscall_spec
: opt_end_time function_name function_arguments '=' expression opt_errno opt_note {
    $$ = (struct syscall_spec *)calloc(1, sizeof(struct syscall_spec));
    $$->end_usecs = $1;
    $$->name = $2;
    $$->arguments = $3;
    $$->result = $5;
    $$->error = $6;
    $$->note = $7;
}
;

opt_end_time
: {
    $$ = -1;
}
| ELLIPSIS time {
    $$ = $2;
}
;

function_name
: WORD {
    $$ = $1;
    current_script_line = yylineno;
}
;

function_arguments
: '(' ')' {
    $$ = NULL;
}
| '(' expression_list ')' {
    $$ = $2;
}
;

expression_list
: expression {
    $$ = new cQueue("expressionList");
    $$->insert((cObject*)$1);
}
| expression_list ',' expression {
    $$ = $1;
    $1->insert($3);
}
;

expression
: ELLIPSIS {
    $$ = new PacketDrillExpression(EXPR_ELLIPSIS);
}
| decimal_integer {
    $$ = $1; }
| hex_integer {
    $$ = $1;
}
| WORD {
    $$ = new PacketDrillExpression(EXPR_WORD);
    $$->setString($1);
}
| MYSTRING {
    $$ = new PacketDrillExpression(EXPR_STRING);
    $$->setString($1);
    $$->setFormat("\"%s\"");
}
| MYSTRING ELLIPSIS {
    $$ = new PacketDrillExpression(EXPR_STRING);
    $$->setString($1);
    $$->setFormat("\"%s\"...");
}
| binary_expression {
    $$ = $1;
}
| array {
    $$ = $1;
}
;



decimal_integer
: INTEGER {
    $$ = new_integer_expression($1, "%ld");
}
;

hex_integer
: HEX_INTEGER {
    $$ = new_integer_expression($1, "%#lx");
}
;

binary_expression
: expression '|' expression {    /* bitwise OR */
    $$ = new PacketDrillExpression(EXPR_BINARY);
    struct binary_expression *binary = (struct binary_expression *) malloc(sizeof(struct binary_expression));
    binary->op = strdup("|");
    binary->lhs = $1;
    binary->rhs = $3;
    $$->setBinary(binary);
}
;

array
: '[' ']' {
    $$ = new PacketDrillExpression(EXPR_LIST);
    $$->setList(NULL);
}
| '[' expression_list ']' {
    $$ = new PacketDrillExpression(EXPR_LIST);
    $$->setList($2);
}
;



opt_errno
: {
    $$ = NULL;
}
| WORD note {
    $$ = (struct errno_spec*)malloc(sizeof(struct errno_spec));
    $$->errno_macro = $1;
    $$->strerror = $2;
}
;

opt_note
: {
    $$ = NULL;
}
| note {
    $$ = $1;
}
;

note
: '(' word_list ')' {
    $$ = $2;
}
;

word_list
: WORD {
    $$ = $1;
}
| word_list WORD {
    asprintf(&($$), "%s %s", $1, $2);
    free($1);
    free($2);
}
;

