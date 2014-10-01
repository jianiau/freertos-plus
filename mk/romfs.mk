ROMDIR = $(DATDIR)/test-romfs
ROMIND = $(DATDIR)/test-romfs-ind
DAT += $(OUTDIR)/$(DATDIR)/test-romfs.o
DAT += $(OUTDIR)/$(DATDIR)/test-romfs-ind.o

$(OUTDIR)/$(ROMDIR).o: $(OUTDIR)/$(ROMDIR).bin $(OUTDIR)/$(ROMDIR).bin.idx
	@mkdir -p $(dir $@)
	@echo "    OBJCOPY "$@
	@$(CROSS_COMPILE)objcopy -I binary -O elf32-littlearm -B arm \
		--prefix-sections '.romfs' $< $@
	@$(CROSS_COMPILE)objcopy -I binary -O elf32-littlearm -B arm \
		--prefix-sections '.romfsind' $(word 2,$^) $(OUTDIR)/$(ROMIND).o 

$(OUTDIR)/$(ROMDIR).bin.idx: $(ROMDIR) $(OUTDIR)/$(TOOLDIR)/mkromfs
	@mkdir -p $(dir $@)
	@echo "    MKROMFS "$@
	@$(OUTDIR)/$(TOOLDIR)/mkromfs -d $< $@
		
$(OUTDIR)/$(ROMDIR).bin: $(ROMDIR) $(OUTDIR)/$(TOOLDIR)/mkromfs
	@mkdir -p $(dir $@)
	@echo "    MKROMFS "$@
	@$(OUTDIR)/$(TOOLDIR)/mkromfs -d $< $@

$(ROMDIR):
	@mkdir -p $@

$(OUTDIR)/%/mkromfs: %/mkromfs.c
	@mkdir -p $(dir $@)
	@echo "    CC      "$@
	@gcc -Wall -o $@ $^
