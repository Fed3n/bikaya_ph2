import os

#Questo SConstruct si aspetta una directory locale con due precise directory uarm_include e umps_include
#E che tutti i source file siano direttamente nella directory locale
#Per ora aggiungere file in directory aggiuntive richiede aggiungere manualmente informazioni al build system

#Inserire elenco di source file da compilare senza estensione separati da spazio
files = 'main p2test_bikaya_v0.2 scheduler handler syscall interrupt init pcb asl auxfun'

files_list = files.split(' ')

#Stringhe da passare nei campi di builder, flags, directories a seconda del target
if 'uarm' in COMMAND_LINE_TARGETS:
	builder = 'arm-none-eabi-'
	ccflags = '-O0 -mcpu=arm7tdmi -DTARGET_UARM=1 -Wall'
	linkflags = '-G0 -nostdlib -T./uarm_include/elf32ltsarm.h.uarmcore.x'
	asflags = '-I./uarm_include'
	dirpath = 'uarm_include'
elif 'umps' in COMMAND_LINE_TARGETS or 'umps2' in COMMAND_LINE_TARGETS:
	builder = 'mipsel-linux-gnu-'
	ccflags = '-ffreestanding -mips1 -mfp32 -DTARGET_UMPS=1 -Wall'
	linkflags = '-nostdlib -T./umps_include/umpscore.ldscript'
	asflags = '-ffreestanding -mips1 -mfp32'
	dirpath = 'umps_include'
else:
	print("Select a target to build")
	print("Usage:\n 'scons uarm' for uarm build\n 'scons umps' or 'scons umps2' for umps build")
	print("Add '-c' option to target of choice to clean files")
	Exit()

#Inizializzazione dell'environment a quello locale per trovare eseguibili
#PATH corrisponde a /usr/local/bin/ dove mi aspetto di trovarli
env = Environment()
env.Append(ENV = {'PATH' : os.environ['PATH']})

#Modifico cross compiler, linker e assembler di default
env.Replace(
	CC = builder+'gcc',
	LINK = builder+'ld',
	AS = builder+'as'
	)
#Aggiungo le flag rispettive
env.Append(
	CCFLAGS = ccflags,
	LINKFLAGS = linkflags,
	ASFLAGS = asflags
	)
#Indico dove trovare gli headers
env.Append(CPPPATH=['.', './'+dirpath])
#I target uarm umps e umps2 in realtà vanno tutti a buildare l'eseguibile kernel
#Ma ovviamente con builder e parametri diversi
env.Alias('uarm', 'kernel')
env.Alias('umps', 'kernel')
env.Alias('umps2', 'kernel')

#Gli oggetti buildati dai file asm sono buildati da un file SConscript nella loro directory
Export('env')
asm_o = SConscript('./'+dirpath+'/SConscript', 'env')

#Prendo i nomi dei source files e mi aspetto siano di tipo .c
src_files = []
for f in files_list:
	src_files.append(f+'.c')

#Compilo i file oggetto dai source file
objs = env.Object(src_files)

#Infine creo l'eseguibile con i file oggetto dai source file e dai file asm
env.Program(target='kernel', source=objs+asm_o)
#Nel caso di umps serve un passaggio aggiuntivo da eseguire manualmente
if 'umps' in COMMAND_LINE_TARGETS or 'umps2' in COMMAND_LINE_TARGETS:
	env.AddPostAction('kernel', 'umps2-elf2umps -k kernel')
	#scons non trova da solo i file da pulire quando li ho creati con un comando manuale
	Clean(['umps', 'umps2'], ['kernel.core.umps','kernel.stab.umps'])
