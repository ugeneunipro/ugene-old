import os
#filtered set diseases. Mapped: 4092
#full set diseases. Mapped: 13255
def getDiseasesMap():
    #data from http://diseases.jensenlab.org/Search
    #f = open ('human_disease_textmining_filtered.tsv', 'r')
    f = open ('human_disease_textmining_full.tsv', 'r')
    
    
    diseasesMap = {}
    while(1):
        line = f.readline()
        if not line:
            break
        line = line.rstrip('\n')
        columns = line.split()
        if len(columns) != 6:
            continue
        id = columns[1]
        disease = columns[3] +'('+columns[5]+')'
        if id not in diseasesMap:
            diseasesMap[id] = []
        diseasesMap[id].append(disease)
        
    f.close()
    return diseasesMap

dmp = getDiseasesMap()

kg_file = open ('knownGene_gene_sym_no_predicted.fg', 'r')
kg_file_new = open ('knownGene_gene_sym_no_predicted_diseases.fg', 'w')
counter = 0
for line in kg_file:
    clms = line.split('\t')
    new_line = line
    if len(clms) >= 15:
        gene_id = clms[0]
        if gene_id in dmp:
            if len(dmp[gene_id]) == 0:
                disLine = "-"
            else:
                disLine = ",".join(dmp[gene_id])
                disLine = "DISEASE: " + disLine
            clms[14] = disLine
            new_line = "\t".join(clms[0:15])
            new_line = new_line + '\n'
            counter += 1
            
    kg_file_new.write(new_line)    
    
    
        
print counter
kg_file.close()
kg_file_new.close()
