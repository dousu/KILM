# -*- encoding : utf-8 -*-

def searchProcess(arg)
  #help
  if arg.include?("-h")
    puts "Format is\nruby search.rb [TargetFolderPass] [compare command]\n[compare command] is using key which is : \nkey        meaning\nexp        expressivity\nsnt        number of sentences\nwrd        number of words\nnum        number of rules\nsdist        language distance\nwdist        word distance\n"
    exit(-1)
  end
  #make folder
  $folder=arg[0]
  $folder += "/" if($folder[$folder.length-1]!="/")
#  puts

  #recognize target file name
  fileArray=Dir.entries($folder).delete_if{|obj| (not obj.include?(".rst"))}
#  puts fileArray.to_s #check
  #puts "regnized directory"

  threads=[]
  tmp=(fileArray.size/$THREADnum).to_i
  tmp1=fileArray.size%$THREADnum
  works=Array.new($THREADnum,tmp)
  tmp1.times{|i| works[i]+=1}
  puts "\n\nNumber of files is #{fileArray.size}"
  puts "WORK SCHEDULE:\n#{works.to_s}"
  puts
  puts
  puts
  tmp2=0
  $THREADnum.times do |i|
    if works[i]>0
#      puts "\n\nSTART THREAD"
#      puts "Thread runs #{works[i]} times"
      th=Thread.new{num=i;value=tmp2;works[num].times{|j| exe_search(fileArray[value+j],num,arg[1].split(" "))}}
      threads.push(th)
      sleep 0.1#確実にSTART THREADのすぐ下にACCESS FILE NAMEがでるようにしただけ(そんな遅くないから意味ないかも.mainのほうがadvantageがでかい)
      tmp2+=works[i]
    end
#    puts tmp2
  end
  threads.each{|t| t.join}
  puts "number of detected file : "+$checkCount.to_s
end
def exe_search(name,th_num,arg)
  #puts "Thread Number #{th_num}"
  #puts "ACCESS FILE NAME:\n"+$folder+name+"\n"
  #$RESULTboxに"#{seed} #{gen}"をkeyとして[#{expresivity},#{number of sentence},#{number of word},#{distance}]の配列を突っ込む
  string=name.split(".rst")[0].split("_") #.rstの除去と_で分割
  seed=string[-2];gen=string[-1]
  key=seed+" "+gen
  
  file=IO.readlines($folder+name).map! do |obj|
    obj.strip!.split(/[=(),]/).delete_if{|el| el==""}
  end
  
  if arg_analyzer(arg,file,seed,gen)
    puts name
    $checkCount+=1
  end  

  #puts file.to_s #->[["#RESULT"], ["BASIC", "41", "87", "23", "29"], ["SDISTM", "0.23221"], ["WDIST", "0"], ["#RESULT"], ["BASIC", "40", "89", "26", "17"], ["SDISTM", "0.285659"], ["WDIST", "0"]]
  #$RESULTbox[key]=[file[1][2].to_f,file[1][3].to_f,file[1][4].to_f,file[1][3].to_f+file[1][4].to_f,file[2][1].to_f,file[3][1].to_f,file[5][2].to_f,file[5][3].to_f,file[5][4].to_f,file[5][3].to_f+file[5][4].to_f,file[6][1].to_f,file[7][1].to_f]
end
def arg_analyzer(arg,filedata,seed,gen)
  count=0
  flag=true
  while(count<arg.length)
    case arg[count]
    when "exp"
      flag=(flag and operation(filedata[1][2].to_i,arg[count+2].to_i,arg[count+1]))
    when "snt"
      flag=(flag and operation(filedata[1][3].to_i,arg[count+2].to_i,arg[count+1]))
    when "wrd"
      flag=(flag and operation(filedata[1][4].to_i,arg[count+2].to_i,arg[count+1]))
    when "num"
      flag=(flag and operation(filedata[1][3].to_i+filedata[1][4].to_i,arg[count+2].to_i,arg[count+1]))
    when "sdist"
      flag=(flag and operation(filedata[2][1].to_i,arg[count+2].to_i,arg[count+1]))
    when "wdist"
      flag=(flag and operation(filedata[3][1].to_i,arg[count+2].to_i,arg[count+1]))
    when "gen"
      flag=(flag and operation(gen.to_i,arg[count+2].to_i,arg[count+1]))
    when "rnd"
      flag=(flag and operation(seed.to_i,arg[count+2].to_i,arg[count+1]))
    else
      flag=10
    end
    count+=3
  end
  flag
end
def operation(val1,val2,operator)
  flag=10
  case operator
  when ">"
    flag=(val1>val2)
  when "<"
    flag=(val1<val2)
  when "=="
    flag=(val1==val2)
  when "<="
    flag=(val1<=val2)
  when ">="
    flag=(val1>=val2)
  when "!="
    flag=(val1!=val2)
  else
    flag=10
  end
  flag
end

$ITERATEnum=100
$msilm="../SOURCE/msilm.exe"
$dictionary="../SOURCE/data.dic"
$THREADnum=10
$checkCount=0;
#puts ARGV
#puts ARGV[1].split(" ")
#exit(-1)
searchProcess(ARGV)
puts "end"

#Example as executing "search.rb"
#ruby search.rb ~/Desktop/MSILMresult/BothBiase/ "exp > 30 snt > 11 wrd > 5 num == 42 sdist < 0.5 wdist < 1 gen >= 10 rnd <= 100"
