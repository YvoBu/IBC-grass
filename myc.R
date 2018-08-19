
########## IBC-Grass + Association type (myc) + plant-soil feedbacks (PSF) ##### evaluation #####

#############################################

# SimIDs are as follows:                    
# 01: mixed, symbiotic                    
# 02: mixed, parasitic                
# 03: mixed, both                      
# 04: obligate mycorrhizal (OM), symbiotic    
# 05: OM, parasitic                      
# 06: OM, both                         
# 07: fakultative mycorrhizal (FM), symbiotic 
# 08: FM, parasitic                      
# 09: FM, both                         
# 16: neutral                               
# 20: OM, neutral                              
# 21: FM, neutral                           

#############################################

# activate packages
library(ggplot2)
library(reshape2)
library(sciplot)

# assure BW-ggplots throughout script (REQUIRES active PACKAGE ggplot!!!)
ggplot <- function(...) { ggplot2::ggplot(...) + theme_bw() }

# change working path
#setwd ("data/out")

Arguments <- commandArgs(trailingOnly=TRUE)

#################### EVALUATION OF BIOMASS AND LOCAL MYCSTATs ####################
NumberOfYears <- Arguments[5]

# read in data to assess biomass:
filename <- paste(Arguments[1], "_aggregated.csv", sep="")
E01 <- read.table(filename, sep=",", header=T)

# create year-based subsets and calculate biomass by means of a loop:

# create table called "E01m": 
# column 1 = "year" with entries 1 to 100
# column 2/3/4 = "shoot"/"root"/"richness"/"OM"/"FM"/"NM" without entry
E01m <- data.frame(Year=(1:NumberOfYears), shoot=NA, root=NA, richness=NA, OM=NA, FM=NA, NM=NA, all=NA)
# now autofill columns 2 to 4 based on Years with results of calculations
# loop goes over "Years"
for (i in (1:NumberOfYears)){
  # number in [] refers to column of respective dataset, here E01
  E01m[i,'shoot'] <- mean(subset(E01,Year==i)[,11])/1000000
  E01m[i,'root'] <- mean(subset(E01,Year==i)[,12])/1000000
  E01m[i,'richness'] <- mean(subset(E01,Year==i)[,7])
  E01m[i,'OM'] <- mean(subset(E01,Year==i)[,19])
  E01m[i,'FM'] <- mean(subset(E01,Year==i)[,20])
  E01m[i,'NM'] <- mean(subset(E01,Year==i)[,21])
  E01m[i,'all'] <- (mean(subset(E01,Year==i)[,19])) + (mean(subset(E01,Year==i)[,20])) + (mean(subset(E01,Year==i)[,21]))
}

# read in data to assess biomass:
filename <- paste(Arguments[3], "_aggregated.csv", sep="")
E02 <- read.table(filename, sep=",", header=T)

# create year-based subsets and calculate biomass by means of a loop:

# create table called "E01m": 
# column 1 = "year" with entries 1 to 100
# column 2/3/4 = "shoot"/"root"/"richness"/"OM"/"FM"/"NM" without entry
E02m <- data.frame(Year=(1:NumberOfYears), shoot=NA, root=NA, richness=NA, OM=NA, FM=NA, NM=NA)
for (i in (1:NumberOfYears)){
  E02m[i,'shoot'] <- mean(subset(E02,Year==i)[,11])/1000000
  E02m[i,'root'] <- mean(subset(E02,Year==i)[,12])/1000000
  E02m[i,'richness'] <- mean(subset(E02,Year==i)[,7])
  E02m[i,'OM'] <- mean(subset(E02,Year==i)[,19])
  E02m[i,'FM'] <- mean(subset(E02,Year==i)[,20])
  E02m[i,'NM'] <- mean(subset(E02,Year==i)[,21])
  E02m[i,'all'] <- (mean(subset(E02,Year==i)[,19])) + (mean(subset(E02,Year==i)[,20])) + (mean(subset(E02,Year==i)[,21]))
}


# RESULTS ARE NOT PLOTTED AT THE MOMENT



#################### EVALUATION OF REGIONAL MYCSTAT & LOCAL AND REGIONAL DIVERSITY ####################
# read in data from conceptual experiment to assess mycstat:
filename <- paste(Arguments[1], "_gamma.csv", sep="")

G01 <- read.table(filename, sep=",", header=T)

# read in data from conceptual experiment to assess diversity:

filename <- paste(Arguments[1], "_div.csv", sep="")

div01 <- read.table(filename, sep=",", header=T)


# create table with MycStat-numbers for all communities
G01M <- data.frame(year=(1:NumberOfYears),OMa=NA, FMa=NA, NMa=NA)
for (i in (1:NumberOfYears)){
  G01M[i,'OMa'] <- round(subset(E01m, Year==i)[,5] * 100 / sum(subset(E01m, Year==i)[,5:7], digits=2))
  G01M[i,'FMa'] <-round(subset(E01m, Year==i)[,6] * 100 / sum(subset(E01m, Year==i)[,5:7], digits=2))
  G01M[i,'NMa'] <-round(subset(E01m, Year==i)[,7] * 100 / sum(subset(E01m, Year==i)[,5:7], digits=2))
}

# calculate standard error for alpha-diversity
G01se <- data.frame(year=(1:NumberOfYears), SEa=NA)
for (i in (1:NumberOfYears)){
  G01se[i,'SEa'] <- se((subset(E01,Year==i)[,7]))
}

G01A<-merge(div01, G01se, by="year")

filename <- paste(Arguments[3], "_gamma.csv", sep="")

G02 <- read.table(filename, sep=",", header=T)

# read in data from conceptual experiment to assess diversity:

filename <- paste(Arguments[3], "_div.csv", sep="")

div02 <- read.table(filename, sep=",", header=T)


G02M <- data.frame(year=(1:NumberOfYears),OMa=NA, FMa=NA, NMa=NA)
for (i in (1:NumberOfYears)){
  G02M[i,'OMa'] <- round(subset(E02m, Year==i)[,5] * 100 / sum(subset(E02m, Year==i)[,5:7], digits=2))
  G02M[i,'FMa'] <-round(subset(E02m, Year==i)[,6] * 100 / sum(subset(E02m, Year==i)[,5:7], digits=2))
  G02M[i,'NMa'] <-round(subset(E02m, Year==i)[,7] * 100 / sum(subset(E02m, Year==i)[,5:7], digits=2))
}

# calculate standard error for alpha-diversity
G02se <- data.frame(year=(1:NumberOfYears), SEa=NA)
for (i in (1:NumberOfYears)){
  G02se[i,'SEa'] <- se((subset(E01,Year==i)[,7]))
}

G02A<-merge(div02, G02se, by="year")

# PLOT DIVERSITY RESULTS FOR SYMBIOSIS AND ALL PFTs

# character help for plotting diversities: α β γ

# create dataframe for plotting with ggplot

# set  "Limit length of lines displayed in the console to": 10000 under Tools/ Global Options/ Code/ Display
# genrate comma separated vector from column
#paste0(G01A$year, collapse=",")
#paste0(G01A$OM, collapse=",")
#paste0(G01A$FM, collapse=",")
#paste0(G01A$NM, collapse=",")

G01Mp <- subset(G01M, year==1 | year==20 | year==39)

m1.data = data.frame (
  year=c(1, 20, 39),
  obligate=c(55,69,70),
  facultative=c(37,31,30),
  none=c(8,0,0)
)

mtm1=melt(m1.data, id.vars=c("year"),
          measure.vars=c("obligate", "facultative", "none"))

pngfilename <- paste(Arguments[1], "_comp.png", sep="")

png(filename=pngfilename)
ggplot(mtm1, aes(x=year, y=value, fill=variable)) +
  theme_bw() +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  geom_bar(position="stack", stat="identity", width = 5.0) +
  scale_fill_manual(values=c("dark gray", "light gray", "black")) +
  theme(legend.position = c(0.7, 0.85)) +
  labs(x="Year",y="PFTs (%)", fill="MycStat:") +
  #scale_x_discrete(limit = c("1", "10", "20")) +
  theme(axis.text = element_text(size=20), axis.title = element_text(size=20), 
        legend.text = element_text(size=20), legend.title = element_text(size=20)) 
dev.off()

G02Mp <- subset(G02M, year==1 | year==20 | year==39)

m2.data = data.frame (
  year=c(1, 20, 39),
  obligate=c(55,69,70),
  facultative=c(37,31,30),
  none=c(8,0,0)
)

mtm2=melt(m2.data, id.vars=c("year"),
          measure.vars=c("obligate", "facultative", "none"))

pngfilename <- paste(Arguments[3], "_comp.png", sep="")

png(filename=pngfilename)
ggplot(mtm2, aes(x=year, y=value, fill=variable)) +
  theme_bw() +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  geom_bar(position="stack", stat="identity", width = 5.0) +
  scale_fill_manual(values=c("dark gray", "light gray", "black")) +
  theme(legend.position = c(0.7, 0.85)) +
  labs(x="Year",y="PFTs (%)", fill="MycStat:") +
  #scale_x_discrete(limit = c("1", "10", "20")) +
  theme(axis.text = element_text(size=20), axis.title = element_text(size=20), 
        legend.text = element_text(size=20), legend.title = element_text(size=20)) 
dev.off()


# use ggplot to show results including standard error (REQUIRES PACKAGES ggplot AND sciplot!!!)
# to plot results from different datasets add the respective dataset to each line or ribbon or whatsoever you plot
pngfilename <- paste(Arguments[1], "_vs_", basename(Arguments[3]), "_01a.png", sep="")

png(filename=pngfilename)
ggplot() +
  theme_bw() +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  # plot data
  geom_line(data=G01A, aes(x = year, y = alpha, colour="steel blue3")) +
  geom_line(data=G02A, aes(x = year, y = alpha, colour="dark orange3")) +
  #geom_line(data=G02A, aes(x = year, y = alpha, colour="black")) +
  # plot SE (use "fill=" for colouring)
  geom_ribbon(data=G01A, aes(x = year, ymin = alpha - SEa,
                             ymax = alpha + SEa), fill="steel blue1", alpha = 0.1) +
  geom_ribbon(data=G02A, aes(x = year, ymin = alpha - SEa,
                             ymax = alpha + SEa), fill="dark orange1", alpha = 0.1) +
  #geom_ribbon(data=G02A, aes(x = year, ymin = alpha - SEa,
  #                           ymax = alpha + SEa), fill="dim gray", alpha = 0.5) +
  scale_colour_manual(name="Feedback:", values = c("steel blue3" = "steel blue3", "dark orange3" = "dark orange3"), #"black" = "black"),
                      labels=c(Arguments[4], Arguments[2])) +
  # act as if you would also add a legend for the ribbons (otherwise the ribbon-colours will change)
  scale_fill_manual(values=c("steel blue1"="steel blue1","dark orange1"="dark orange1")) + #"dim gray" = "dim gray")) +
  # but actually exclude it from the legend
  guides(fill=FALSE) +
  # change position of the legend (distance from the left, and the bottom)
  theme(legend.position = c(0.8, 0.85)) +
  # add lables for x- and y-axis
  labs(x="Year",y="α-diversity") +
  # change font size
  theme(axis.text = element_text(size=20), axis.title = element_text(size=20), 
        legend.text = element_text(size=20), legend.title = element_text(size=20)) 
# remove background grid
# theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) 
dev.off()

pngfilename <- paste(Arguments[1], "_vs_", basename(Arguments[3]), "_01b.png", sep="")

png(filename=pngfilename)
ggplot() +
  theme_bw() +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  geom_line(data=G01A, aes(x = year, y = beta, colour="steel blue3"), size=1.5) +
  #geom_ribbon(data=G16A, aes(x = year, ymin = beta - SDEb,
  #                           ymax = beta + SDEb), fill="steel blue1", alpha = 0.5) +
  geom_line(data=G02A, aes(x = year, y = beta, colour="dark orange3"), size=1.5) +
  #geom_ribbon(data=G01A, aes(x = year, ymin = beta - SDEb,
  #                           ymax = beta + SDEb, fill="dark orange1") , alpha = 0.5) +
  scale_colour_manual(name="Feedback:", values = c("steel blue3" = "steel blue3", "dark orange3" = "dark orange3"), 
                      labels= c(Arguments[4], Arguments[2])) +
  #scale_fill_manual(values=c("steel blue1"="steel blue1","dark orange1"="dark orange1")) +
  #guides(fill=FALSE) +
  theme(legend.position = c(0.8, 0.15)) +
  labs(x="Year",y="β-diversity") +
  theme(axis.text = element_text(size=20), axis.title = element_text(size=20), 
        legend.text = element_text(size=20), legend.title = element_text(size=20)) 
dev.off()

pngfilename <- paste(Arguments[1], "_vs_", basename(Arguments[3]), "_01c.png", sep="")

png(filename=pngfilename)
ggplot() +
  theme_bw() +
  theme(panel.grid.major = element_blank(), panel.grid.minor = element_blank()) +
  geom_line(data=G01A, aes(x = year, y = gamma, colour="steel blue3"), size=1.5) +
  #geom_ribbon(data=G16A, aes(x = year, ymin = gamma - SDEg,
  #                           ymax = gamma + SDEg), fill="steel blue1", alpha = 0.1) +
  geom_line(data=G02A, aes(x = year, y = gamma, colour="dark orange3"), size=1.5) +
  #geom_ribbon(data=G01A, aes(x = year, ymin = gamma - SDEg,
  #                           ymax = gamma + SDEg, fill="dark orange1") , alpha = 0.1) +
  scale_colour_manual(name="Feedback:", values = c("steel blue3" = "steel blue3", "dark orange3" = "dark orange3"), 
                      labels= c(Arguments[4], Arguments[2])) +
  #scale_fill_manual(values=c("steel blue1"="steel blue1","dark orange1"="dark orange1")) +
  #guides(fill=FALSE) +
  theme(legend.position = c(0.8, 0.85)) +
  labs(x="Year",y="γ-diversity") +
  theme(axis.text = element_text(size=20), axis.title = element_text(size=20), 
        legend.text = element_text(size=20), legend.title = element_text(size=20)) 
dev.off()


