# -*- encoding : utf-8 -*-

def main(arg)
  #prohibit dictionary option
  if (arg.include?("--dictionary") or arg.include?("-d"))
    puts "DON'T SET DICTIONARY FILE"
    puts "FORCING EXIT"
    exit(-1)
  end
  #prohibit folder option
  if arg.include?("--path")
    puts "DON'T SET OUTPUT FOLDER"
    puts "FORCING EXIT"
    exit(-1)
  end
  #prohibit seed option
  if (arg.include?("--random-seed") or arg.include?("-r"))
    puts "DON'T SET RANDOM SEED"
    puts "FORCING EXIT"
    exit(-1)
  end
  #help
  if arg.include?("-h")
    result=exe_msilm("-h")
    while line=result.gets
      puts line
    end
    result.close
    puts "Format is\nruby exe.rb [OutputFolderPass] [MSILM OPTION]\nOutput folder option of \"MSILM.exe\" is automatically set by program."
    exit(-1)
  end
  #make folder or clean folder
  $folder=arg[0]
  $folder += "/" if($folder[$folder.length-1]!="/")
  if not File.exist?($folder)
    Dir.mkdir($folder)
  else
    Dir.foreach($folder) do |fn|
      if not File.directory?($folder+fn)
        File.delete($folder+fn)
      end
    end
  end

  #folder option
  command=arg[1..arg.length].join(" ") + " --path " + $folder
  puts "\nMSILM.exe Command Option : "+command
  
  puts "Program runs MSILM #{$ITERATEnum} times." if $ITERATEnum>0
  threads=[]
  tmp=($ITERATEnum/$THREADnum).to_i
  tmp1=$ITERATEnum%$THREADnum
  works=Array.new($THREADnum,tmp)
  tmp1.times{|i| works[i]+=1}
  puts #tmp.to_s
  puts #tmp1.to_s
  puts #works.to_s
  tmp2=0
  $THREADnum.times do |i|
    if works[i]>0
      puts "\n\nSTART THREAD"
      puts "Thread runs #{works[i]} times"
      th=Thread.new{num=i;seed=tmp2;works[num].times{|j| exe_msilm(command+" -r #{seed+j}")}}
      threads.push(th)
      sleep 0.1#確実にSTART THREADのすぐ下にEXECUTE COMMANDがでるようにしただけ.スレッドをたちあげてる間に,次のSTART　THREADが出る場合がある
      tmp2+=works[i]
    end
#    puts tmp2
  end
  threads.each{|t| t.join}
  
end
def exe_msilm(command)
  puts "\nEXECUTE COMAND:\n"+$msilm+" -d "+$dictionary+" "+command
  system($msilm+" -d "+$dictionary+" "+command)
#  `"$msilm+" -d "+$dictionary+" "+command"`
#  open("| "+$msilm+" -d "+$dictionary+" "+command)
end

$ITERATEnum=ARGV[0]
$msilm="../LEILA/msilm.exe"
$dictionary="../LEILA/data.dic"
$THREADnum=10
$RESULTbox=Hash.new
$COUNTINGbox=Array.new
main(ARGV[1..ARGV.size])
#Example as option of MSILM
# -p --path ../result/ -l -u 0.50 --analyze -g 100 --interspace-analysis 10 --term 0.8 -m 3 --interspace-logging 10 --symmetry --mutual-exclusivity
#Example as executing "exe.rb"
#ruby exe.rb [ITERATEnum] ~/Desktop/MSILMresult/BothBiase/ -u 0.50 -g 100 --keep-random-rule --unique-utterance --analyze --interspace-analysis 10 --logging --interspace-logging 10 --term 0.8 -m 10 --symmetry --mutual-exclusivity
