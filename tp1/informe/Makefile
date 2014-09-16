TRASH= informe.aux informe.log informe.out informe.toc informe.idx
TARGET= informe.df

.PHONY: clean all

all: informe.pdf

# El latex se compila dos veces seguidas, la primera es para que se arme bien el índice.
TEX_PARAMS= -interaction=nonstopmode
informe.pdf: informe.tex
	( pdflatex $(TEX_PARAMS) informe.tex &&\
	pdflatex $(TEX_PARAMS) informe.tex ) || (echo 'ERROR.'&&rm informe.pdf)

# No borro el informe.pdf
clean:
	rm -f $(TRASH)
