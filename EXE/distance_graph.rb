# -*- encoding : utf-8 -*-

def graphILM(arg)
  
  $BASE_DIR=$BASE_DIR+$PREFIX
  $folder=arg
  
  filelist=Dir.entries($folder).delete_if{|obj| (not obj.include?("dist.ave"))}
  
  filelist=filelist.sort_by{|a| [a.size,a]}
  #puts filelist
  filelist.map!{|obj| $folder+obj}
  
  filelist.each do |file|
    if(file !~ /.+\.dist\.ave/)
      puts "extname unknown"
      exit(-1)
    end
    if not File.exist?(file)
      puts "file : #{file} don't exist"
    end
  end
  
  IO.popen("/usr/bin/gnuplot","w") do |gp|
    gp.puts('set terminal "wxt"')
    gp.puts('set xl"Generation"')
    flag=false
    filelist.each do |file|
      if((file.split("/")[-1].split(".")[0].split("_")[-1]).to_i%$GAP==0)
        title=file.split("/")[-1].split(".")[0].delete("_")
        if flag
          gp.puts("replot \"#{file}\" using 1:2 w l title \'#{title}\'")
        else
          gp.puts("plot \"#{file}\" using 1:2 w l title \'#{title}\'")
          flag=true
        end
      end
    end
    gp.puts("set xr[0:#{$MAX_GEN}]")
    gp.puts("set yr[0:#{$MAX_DIST}]")
    gp.puts("set yl\"Language Distance\"")
    gp.puts("set key outside")
    gp.puts("set terminal postscript #{$PSTSCRPT_POINT} eps enhanced color")
    gp.puts("set output \"#{$BASE_DIR}language_distance2.eps\"")
    gp.puts('replot')
    gp.puts("set key top")
    
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
$PREFIX=ARGV[1]
$GAP=10

graphILM(ARGV[0])

#ruby graph.rb "prefix of output filename" "filename[.cnt.ave]"
