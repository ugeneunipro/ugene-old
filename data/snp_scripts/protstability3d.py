#v 1.0
import sys, os, time, re
import urllib, urllib2 
from optparse import OptionParser
from HTMLParser import HTMLParser
from xml.dom.minidom import parseString

# Define a function for the thread

class NoRedirectHandler(urllib2.HTTPRedirectHandler):
    def http_error_302(self, req, fp, code, msg, headers):
        infourl = urllib.addinfourl(fp, headers, req.get_full_url())
        infourl.status = code
        infourl.code = code
        return infourl
    http_error_300 = http_error_302
    http_error_301 = http_error_302
    http_error_303 = http_error_302
    http_error_307 = http_error_302

def prepare_optparser():
    """Prepare optparser object. New options will be added in this
    function first.
    """
    usage = "usage: %prog <-i STRING -c STRING -p INT -t STRING> [options]"
    description = "Transcription factors binds to two DNAs: SNPs and\or site-directed mutations."

    optparser = OptionParser(version="%prog v1.0", description=description, usage=usage, add_help_option=False)
    optparser.add_option("-h","--help",action="help",help="Show this help message and exit.")
    
    optparser.add_option("-i","--pdb",dest="pdbId",type="string",
                         help="Enter a PDB ID.")

    optparser.add_option("-c", "--chain", dest="chain",type="string",
                         help="Enter chain.")
    
    optparser.add_option("-p", "--pos", dest="mutPos",type="int",
                         help="Enter mutation position.")

    optparser.add_option("-t", "--type", dest="aminoType",type="string",
                         help="Enter amino acid type.")
                                        
    return optparser

def opt_validate(optparser):
    """Validate options from a OptParser object.

    Return: Validated options object.
    """
    (options,args) = optparser.parse_args()
        
    if not (options.chain and options.pdbId and options.mutPos and options.aminoType):
        optparser.print_help()
        sys.exit(1)
 
    print
    return options

class Protstability3d:
    def __init__(self, options):
        self.pdbId = options.pdbId
        self.mutPos = options.mutPos
        self.chain = options.chain
        self.aminoType = options.aminoType
        self.url = 'http://www-bionet.sscc.ru/psd2/rest.php?tool=protstability3d'

    def sendQuery(self):
        opener = urllib2.build_opener(NoRedirectHandler())
        urllib2.install_opener(opener)

        # do POST
                                          
        params = urllib.urlencode({'pdb': self.pdbId, 'chain': self.chain,  'pos_mut': self.mutPos, 'mutation': self.aminoType})
        req = urllib2.Request(self.url,  params)

        rsp = urllib2.urlopen(req) 

        content = rsp.read()
        res = content.find('qid=')
        content = content[res+5:]
        res = content.find('"')
        content = content[:res]
        self.url = "http://www-bionet.sscc.ru/psd2/rest.php?tool=protstability3d&q=%s" %content
        self.getResult()      

    def getResult(self):
        opener = urllib2.build_opener(NoRedirectHandler())
        urllib2.install_opener(opener)
        content = "working"

        while content.find('working') >= 0:
            req = urllib2.Request(self.url)
            rsp = urllib2.urlopen(req) 
            content = rsp.read()
            time.sleep(0.1)
        self.parseOutput(content)
    def parseOutput(self, content):
        xmldoc = parseString(content)
        
        itemlist = xmldoc.getElementsByTagName('stability')
        if itemlist.length <= 0:
            print "Error"
            sys.exit(1)
 
        res = self.pdbId
        res += '[' +  str(self.mutPos) + ',' + self.aminoType + ']'
        res += ':' +  str(itemlist[0].firstChild.nodeValue)
        print res                           
def main():
    opts=opt_validate(prepare_optparser())
    g = Protstability3d(opts)
    g.sendQuery()
if __name__ == "__main__":
    main()

