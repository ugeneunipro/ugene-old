#!/usr/bin/env python
"""
#v_1.00
"""
import sys, os, time, re
import urllib, urllib2, cookielib
from optparse import OptionParser

# print current time and information on screen
def Info(infoStr):
    print "[%s] %s" %(time.strftime('%H:%M:%S'), infoStr)

def prepare_optparser():
    """Prepare optparser object. New options will be added in this
    function first.
    """
    usage = "usage: %prog <-f STRING -s STRING> [options]"
    description = "Transcription factors binds to two DNAs: SNPs and\or site-directed mutations."

    optparser = OptionParser(version="%prog v1.02", description=description, usage=usage, add_help_option=False)
    optparser.add_option("-h","--help",action="help",help="Show this help message and exit.")
    
    optparser.add_option("-f", "--snpf1",dest="firstSeq",type="string",
                         help="Select a nucleotide sequence of the SNP variant #1.")

    optparser.add_option("-s", "--snpf2",dest="secondSeq",type="string",
                         help="Select a nucleotide sequence of the SNP variant #2.")
    
    optparser.add_option("--snpdm1",dest="snpdm1",type="int",
                         help="""Select How the unknown binds to DNA#1.\n
                         Enter integer from 0 to 5\n
                         default:1""", default=1)

    optparser.add_option("--snpdm2",dest="snpdm2",type="int",
                         help="""Select How the unknown binds to DNA#2.\n
                         Enter integer from 0 to 5\n
                         default:2""", default=2)
                                        
    optparser.add_option("--sign",dest="significance",type="int",
                         help="Select significance. Enter integer from 0 to 16", default=10)
    return optparser

def opt_validate(optparser):
    """Validate options from a OptParser object.

    Return: Validated options object.
    """
    (options,args) = optparser.parse_args()
 
    if options.firstSeq and '"' in options.firstSeq:
        options.firstSeq = options.firstSeq.replace('"','')
    if options.secondSeq and '"' in options.secondSeq:
        options.secondSeq = options.secondSeq.replace('"','')
        
    if not options.firstSeq and not options.secondSeq:
        optparser.print_help()
        sys.exit(1)

    if not 0 <= options.snpdm1 <= 5 and 0 <= options.snpdm2 <= 5:
        Info('Wanring: snpdm values must be in range from 0 to 5.')
        sys.exit(1)
    if not 0 <= options.significance <= 16:
        Info('Wanring: Significance value must be in range from 0 to 16.')
        sys.exit(1)
 
    print
    return options

class RSNP:
    def __init__(self, options):
        self.firstSeq = options.firstSeq
        self.secondSeq = options.secondSeq
        self.snpdm1 = options.snpdm1
        self.snpdm2 = options.snpdm2 
        self.significance = options.significance
        self.opts_string = "# Argument List:\n" +\
                           "# first sequence = %s\n" %options.firstSeq +\
                           "# second sequence = %s\n" %options.secondSeq +\
                           "# Unknown binds to DNA#1 = %d\n" %options.snpdm1 +\
                           "# Unknown binds to DNA#2 = %d\n" %options.snpdm2 +\
                           "# Significance = %d\n" %options.significance
        self.firstSeqFromFile = ""
        self.secondSeqFromFile = ""
        self.absentSides = []
        self.presentSides = []
        self.noIdeaSides = []

    def sendQuery(self):
        cookie_jar = cookielib.CookieJar()
        opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cookie_jar))
        urllib2.install_opener(opener)

        # do POST
        url = 'http://samurai.bionet.nsc.ru/cgi-bin/03/programs/rsnp_lin/rsnpd.pl'
        req = urllib2.Request(url, "b1=Calculate&snpf1={0}&snpf2={1}&SNPDM_0={2}&SNPDM_1={3}&DSL={4}".format(self.firstSeq, self.secondSeq, self.snpdm1, self.snpdm2, self.significance))
        rsp = urllib2.urlopen(req)
        content = rsp.read()
        res = content.find('PRESENT:</b>&nbsp;')
        content = content[res+18:]
        res = content.find('<P align')
        content = content[:res]
        out = content.split()
        for item in out:
            if item:
                print("PRESENT:"+item)
def main():
    opts=opt_validate(prepare_optparser())
    g = RSNP(opts)
    g.sendQuery()

if __name__ == "__main__":
    main()