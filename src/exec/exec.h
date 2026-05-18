#ifndef EXEC_H
#define EXEC_H

#include "parse/cmd/cmd.h"
#include "parse/pipeline/pipeline.h"
#include "parse/separator/separator.h"

int execCmd(Cmd_t* cmd);
int execPipeline(Pipeline_t* pipeline);
int execSeparator(Separator_t* separator);

#endif
