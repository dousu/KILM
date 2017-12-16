# -*- encoding : utf-8 -*-

def graphILM(filelist)
  
  $BASE_DIR=$BASE_DIR+$PREFIX
  
  filelist.sort!
  
  filelist.each do |file|
    if(file !~ /.+\.cnt\.ave/)
      puts "extname unknown"
      exit(-1)
    end
    if not File.exist?(file)
      puts "file : #{file} don't exist"
    end
  end
  
  IO.popen("/usr/bin/gnuplot","w") do |gp|
  
    gp.puts('set xl"Generation"')
    gp.puts('set terminal "wxt"')
    flag=false
    filelist.each do |file|
      title=file.split("/")[-1].split(".")[0]
      if flag
        gp.puts("replot \"#{file}\" using 1:2 w l title \'#{title}\'")
      else
        gp.puts("plot \"#{file}\" using 1:2 w l title \'#{title}\'")
        flag=true
      end
    end
    gp.puts("set xr[0:#{$MAX_GEN}]")
    gp.puts("set yr[0:#{$MAX_EXP}]")
    gp.puts("set yl\"Expressible meanings\"")
    gp.puts("set key bottom")
    gp.puts("set terminal postscript #{$PSTSCRPT_POINT} eps enhanced color")
    gp.puts("set output \"#{$BASE_DIR}expressivity.eps\"")
    gp.puts('replot')
    gp.puts("set key top")
    
    gp.puts('set terminal "wxt"')
    flag=false
    filelist.each do |file|
      title=file.split("/")[-1].split(".")[0]
      if flag
        gp.puts("replot \"#{file}\" using 1:3 w l title \'#{title}\'")
      else
        gp.puts("plot \"#{file}\" using 1:3 w l title \'#{title}\'")
        flag=true
      end
    end
    gp.puts("set xr[0:#{$MAX_GEN}]")
    gp.puts("set yr[0:#{$MAX_NUM}]")
    gp.puts("set yl\"Number of sentences\"")
    gp.puts("set terminal postscript #{$PSTSCRPT_POINT} eps enhanced color")
    gp.puts("set output \"#{$BASE_DIR}number_of_sentences.eps\"")
    gp.puts('replot')
    
    gp.puts('set terminal "wxt"')
    flag=false
    filelist.each do |file|
      title=file.split("/")[-1].split(".")[0]
      if flag
        gp.puts("replot \"#{file}\" using 1:4 w l title \'#{title}\'")
      else
        gp.puts("plot \"#{file}\" using 1:4 w l title \'#{title}\'")
        flag=true
      end
    end
    gp.puts("set xr[0:#{$MAX_GEN}]")
    gp.puts("set yr[0:#{$MAX_NUM}]")
    gp.puts("set yl\"Number of words\"")
    gp.puts("set terminal postscript #{$PSTSCRPT_POINT} eps enhanced color")
    gp.puts("set output \"#{$BASE_DIR}number_of_words.eps\"")
    gp.puts('replot')
    
    gp.puts('set terminal "wxt"')
    flag=false
    filelist.each do |file|
      title=file.split("/")[-1].split(".")[0]
      if flag
        gp.puts("replot \"#{file}\" using 1:5 w l title \'#{title}\'")
      else
        gp.puts("plot \"#{file}\" using 1:5 w l title \'#{title}\'")
        flag=true
      end
    end
    gp.puts("set xr[0:#{$MAX_GEN}]")
    gp.puts("set yr[0:#{$MAX_NUM}]")
    gp.puts("set yl\"Number of rules\"")
    gp.puts("set terminal postscript #{$PSTSCRPT_POINT} eps enhanced color")
    gp.puts("set output \"#{$BASE_DIR}number_of_rules.eps\"")
    gp.puts('replot')
    
    gp.puts('set terminal "wxt"')
    flag=false
    filelist.each do |file|
      title=file.split("/")[-1].split(".")[0]
      if flag
        gp.puts("replot \"#{file}\" using 1:6 w l title \'#{title}\'")
      else
        gp.puts("plot \"#{file}\" using 1:6 w l title \'#{title}\'")
        flag=true
      end
    end
    gp.puts("set xr[0:#{$MAX_GEN}]")
    gp.puts("set yr[0:*]")
    gp.puts("set yl\"Language Distance\"")
    gp.puts("set terminal postscript #{$PSTSCRPT_POINT} eps enhanced color")
    gp.puts("set output \"#{$BASE_DIR}language_distance.eps\"")
    gp.puts('replot')
  
    gp.puts('set terminal "wxt"')
    flag=false
    filelist.each do |file|
      title=file.split("/")[-1].split(".")[0]
      if flag
        gp.puts("replot \"#{file}\" using 1:7 w l title \'#{title}\'")
      else
        gp.puts("plot \"#{file}\" using 1:7 w l title \'#{title}\'")
        flag=true
      end
    end
    gp.puts("set xr[0:#{$MAX_GEN}]")
    gp.puts("set yr[0:*]")
    gp.puts("set yl\"Number of characters in a utterance\"")
    gp.puts("set terminal postscript #{$PSTSCRPT_POINT} eps enhanced color")
    gp.puts("set output \"#{$BASE_DIR}number_utterance_words.eps\"")
    gp.puts('replot')

    gp.puts('set terminal "wxt"')
    flag=false
    filelist.each do |file|
      title=file.split("/")[-1].split(".")[0]
      if flag
        gp.puts("replot \"#{file}\" using 2:5 w l title \'#{title}\'")
      else
        gp.puts("plot \"#{file}\" using 2:5 w l title \'#{title}\'")
        flag=true
      end
    end
    gp.puts("set xr[0:*]")
    gp.puts("set yr[0:100]")
    gp.puts("set xl\"Expressivity\"")
    gp.puts("set yl\"Number of rules\"")
    gp.puts("set terminal postscript #{$PSTSCRPT_POINT} eps enhanced color")
    gp.puts("set output \"#{$BASE_DIR}compositionality.eps\"")
    gp.puts('replot')
  
    gp.puts('exit')
    gp.close()
  end
  
end

$MAX_GEN=100
$MAX_EXP=100
$MAX_NUM=100
$MAX_DIST=1
$BASE_DIR="~/Desktop/"
$PSTSCRPT_POINT=20
$PREFIX=ARGV[0]

graphILM(ARGV[1..ARGV.size])

#ruby graph.rb "prefix of output filename" "filename[.cnt.ave]"
