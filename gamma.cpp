#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <string>

std::map<long, std::set<std::string> > gamma;
std::map<long, std::map<long, std::set<std::string> > > alpha;

int main(int argc, char**argv) {
   if (argc > 1) {
      long linecount = 0;
      std::string   line;
      std::vector<std::string>  headerlist;
      std::ifstream gammafile(argv[1]);
      
      while (getline(gammafile, line).good()) {
         if (linecount == 0) {
            std::string header;
            size_t      start       = 0;
            size_t      end         = 0;

                do {
                    end=line.find_first_of(",", start);
                    if (end != std::string::npos) {
                        header=line.substr(start, end-start);
                    } else {
                        header=line.substr(start);
                    }
                    /*
                     * trim front
                     */
                    while (header[0]==' ') header.erase(0, 1);
                    /*
                     * if extraheader has a size after trimming we can add it to the
                     * list of extra headers
                     */
                    if (header.size()>0) {
                        headerlist.push_back(header);
                    }
                    /*
                     * if we are not at end of search we skip a character.
                     * At end we do nothing and let the loop condition stop
                     * the loop.
                     */
                    if (end == std::string::npos) {
                        start = end;
                    } else {
                        start = end + 1;
                    }
                } while (end!=std::string::npos);

         } else {
            std::string header;
            size_t      start       = 0;
            size_t      end         = 0;
            int             colcount = 0;
            long            count;
            long            year;
            long            run;

                do {
                    end=line.find_first_of(",", start);
                    if (end != std::string::npos) {
                        header=line.substr(start, end-start);
                    } else {
                        header=line.substr(start);
                    }
                    /*
                     * trim front
                     */
                    while (header[0]==' ') header.erase(0, 1);
                    /*
                     * if extraheader has a size after trimming we can add it to the
                     * list of extra headers
                     */
                    if (header.size()>0) {
                        count = strtol(header.c_str(), 0, 0);
                        switch (colcount) {
                        case 0:
                            run = count;
                            break;
                        case 1:
                            year = count;
                            break;
                        default:
                            if (count > 0) {
                                std::map<long, std::map<long, std::set<std::string> > >::iterator ai = alpha.find(year);
                                if (ai != alpha.end()) {
                                    ai->second[run].insert(headerlist[colcount]);
                                } else {
                                    std::map<long, std::set<std::string> > newyear;
                                    
                                    newyear[run].insert(headerlist[colcount]);
                                    alpha.insert(std::pair<long, std::map<long, std::set<std::string> > >(year, newyear));
                                }
                                gamma[year].insert(headerlist[colcount]);
                            }
                            break;
                        }
                        colcount++;
                    }
                    /*
                     * if we are not at end of search we skip a character.
                     * At end we do nothing and let the loop condition stop
                     * the loop.
                     */
                    if (end == std::string::npos) {
                        start = end;
                    } else {
                        start = end + 1;
                    }
                } while ((end!=std::string::npos) && (headerlist[colcount] != "OMcount"));

         }
         linecount++;
      }
      std::map<long, std::set<std::string> >::iterator gi;
      std::cerr << "year,gamma,alpha,beta\n";
      size_t i;
      
      double gsum = 0;
      double gav  = 0;
 
      for (i = 1; i<= gamma.size(); i++) {
          gsum+= gamma[i].size();
      }
      gav = gsum/gamma.size();

      for (i = 1; i<= gamma.size(); i++) {
          std::cerr << i << "," << gamma[i].size() << ",";

          double asum = 0;
          size_t a;
          for (a = 0; a < alpha[i].size(); ++a) {
              asum+=alpha[i][a].size();
          }
          double thealpha = asum/a;
          std::cerr << thealpha << "," << gamma[i].size()/thealpha;
          std::cerr << std::endl;
      }
   }
   return 0;
}

