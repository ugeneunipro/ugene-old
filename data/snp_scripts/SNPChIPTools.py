#v 1.0
import sys, os 
import urllib, urllib2 
from optparse import OptionParser
import csv
from xml.dom.minidom import parseString
import StringIO

def prepare_optparser():
    """Prepare optparser object. New options will be added in this
    function first.
    """
    usage = "usage: %prog <-i STRING> [options]"
    description = "SNPChIPTools."

    optparser = OptionParser(version="%prog v1.0", description=description, usage=usage, add_help_option=False)
    optparser.add_option("-h","--help",action="help",help="Show this help message and exit.")
    
    optparser.add_option("-i","--id",dest="id",type="string",
                         help="Enter ID.")
    
    optparser.add_option("--ug", dest="ug",type="string",
                         help="Enter UG('true' or 'false'). Default: 'true'")

    return optparser

def opt_validate(optparser):
    """Validate options from a OptParser object.

    Return: Validated options object.
    """
    (options,args) = optparser.parse_args()
        
    if not (options.id):
        optparser.print_help()
        sys.exit(1)
 
    return options

class Snp2pdbsite:
    def __init__(self, options):
        self.id = options.id
        if not options.ug:
            options.ug = "true" 
        self.ug = options.ug
        self.url = 'http://lungry1.bionet.nsc.ru/cgi-bin/SNPProject/SNPChIPTools.cgi'

    def sendQuery(self):
        opener = urllib2.build_opener()
        urllib2.install_opener(opener)
        # do POST

        params = urllib.urlencode({'id_SNP': self.id, 'UG': self.ug})
        req = urllib2.Request(self.url,  params)

        rsp = urllib2.urlopen(req) 

        content = rsp.read()
	self.parseOutput(content)

    def parseOutput(self, content):
        sidx = content.find('checked onclick="submit()"> ')
        eidx = content.find('></td></tr></table></form>')
        if eidx > sidx and eidx != -1 and sidx != -1:
            content = content[sidx:eidx]
        csv_dict = csv.DictReader(StringIO.StringIO(content), delimiter='\t', quotechar='|')
        id_score = {}
        for row in csv_dict:
            score = row[" score"]
            if score is None:
                continue
            score = score.strip()
            id = row[" \xd2\xd4 "]
            if id is None:
                continue
            id = id.strip()
            if not (score is None):
                id_score[id] = score
                #print("SNP_CHIP:{0}[{1}];".format(id, score))
        if len(id_score) != 0:
            out_str = 'SNP_CHIP:'
            for id in id_score:
                out_str = out_str + str(id) + '[' + str(id_score[id]) + '];'
            print out_str
        
def main():
    opts=opt_validate(prepare_optparser())
    g = Snp2pdbsite(opts)
    g.sendQuery()
if __name__ == "__main__":
    main()

