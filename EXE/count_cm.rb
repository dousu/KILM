# -*- encoding : utf-8 -*-

def countProcess(arg)
  #help
  if arg.include?("-h")
    puts "Format is\nruby count.rb [TargetFolderPass]\nAll file in the folder are counted per each generation by program."
    exit(-1)
  end
  #make folder
  $folder=arg[0]
  $folder += "/" if($folder[$folder.length-1]!="/")
  #result file name
  $CountingFileName=$folder.split("/")[-1]+".cnt.ave"
  $CountingFileName2=$folder.split("/")[-1]
#  puts $CountingFileName #check

  Dir.foreach($folder+"../") do |fn|
    if not File.directory?($folder+"../"+fn)
      File.delete($folder+"../"+fn)
    end
  end

  #recognize target file name
  fileArray2=Dir.entries($folder).delete_if{|obj| (not obj.include?("distance.rst"))}
  fileArray=Dir.entries($folder).delete_if{|obj| (not obj.include?(".rst"))}.delete_if{|obj| obj.include?("distance.rst")}

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
      puts "\n\nSTART THREAD"
      puts "Thread runs #{works[i]} times"
      th=Thread.new{num=i;value=tmp2;works[num].times{|j| exe_counting(fileArray[value+j],num)}}
      threads.push(th)
      sleep 0.1#確実にSTART THREADのすぐ下にACCESS FILE NAMEがでるようにしただけ(そんな遅くないから意味ないかも.mainのほうがadvantageがでかい)
      tmp2+=works[i]
    end
#    puts tmp2
  end
  threads.each{|t| t.join}

  threads=[]
  tmp=(fileArray2.size/$THREADnum).to_i
  tmp1=fileArray2.size%$THREADnum
  works=Array.new($THREADnum,tmp)
  tmp1.times{|i| works[i]+=1}
  puts "\n\nNumber of files is #{fileArray2.size}"
  puts "WORK SCHEDULE:\n#{works.to_s}"
  puts
  puts
  puts
  tmp2=0
  $THREADnum.times do |i|
    if works[i]>0
      puts "\n\nSTART THREAD"
      puts "Thread runs #{works[i]} times"
      th=Thread.new{num=i;value=tmp2;works[num].times{|j| exe_counting2(fileArray2[value+j],num)}}
      threads.push(th)
      sleep 0.1#確実にSTART THREADのすぐ下にACCESS FILE NAMEがでるようにしただけ(そんな遅くないから意味ないかも.mainのほうがadvantageがでかい)
      tmp2+=works[i]
    end
  end
  threads.each{|t| t.join}
  
  #puts $RESULTbox #check
  #puts $RESULTbox.size #check
  
  numcounter=Array.new
  
  $RESULTbox.each_pair do |key, ar|
    index=key.split(" ")[-1].to_i
    if $COUNTINGbox[index]==nil
      $COUNTINGbox[index]=ar.dup#.map!{|obj| obj/$ITERATEnum}
      numcounter[index]=1
    else
      ar.each_with_index do |el,i|
        $COUNTINGbox[index][i]+=el#/$ITERATEnum
      end
      numcounter[index]+=1
    end
  end
  
  numcounter2=Array.new
  $RESULTbox2.each_pair do |key, ar|
    index=key.split(" ")[-1].to_i
    if $COUNTINGbox2[index]==nil
      new_arr=[]
      ar.each_with_index do |el,i|
        new_arr[el[0].to_i]=el[1]
      end
      $COUNTINGbox2[index]=new_arr.dup
      numcounter2[index]=1
    else
      ar.each_with_index do |el,i|
        if $COUNTINGbox2[index][el[0].to_i]!=nil
          $COUNTINGbox2[index][el[0].to_i]+=el[1]
        end
      end
      numcounter2[index]+=1
    end
  end

  $COUNTINGbox.each_with_index do |ar,i|
    if(numcounter[i]!=nil)
      ar.map!{|obj| obj/numcounter[i]}
    end
  end
  $ITERATEnum=$RESULTbox.size

  $COUNTINGbox2.each_with_index do |ar,i|
    if(numcounter2[i]!=nil)
      ar.map!{|obj| obj/numcounter2[i] if obj!=nil}
    end
  end
  
  #output to file
  #puts $COUNTINGbox.to_s #check
  outputCOUNTINGbox;
  outputCOUNTINGbox2;
  
end
def exe_counting(name,th_num)
  puts "Thread Number #{th_num}"
  puts "ACCESS FILE NAME:\n"+$folder+name+"\n"
  #$RESULTboxに"#{seed} #{gen}"をkeyとして[#{expresivity},#{number of sentence},#{number of word},#{distance}]の配列を突っ込む
  string=name.split(".rst")[0].split("_") #.rstの除去と_で分割
  seed=string[-2];gen=string[-1]
  key=seed+" "+gen
  
  file=IO.readlines($folder+name).map! do |obj|
    obj.strip!.split(/[=(),]/).delete_if{|el| el==""}
  end
  #puts file.to_s #->[["#RESULT"], ["BASIC", "41", "87", "23", "29"], ["SDISTM", "0.23221"], ["WDIST", "0"], ["#RESULT"], ["BASIC", "40", "89", "26", "17"], ["SDISTM", "0.285659"], ["WDIST", "0"]]
  $RESULTbox[key]=[file[1][2].to_f,file[1][3].to_f,file[1][4].to_f,file[1][3].to_f+file[1][4].to_f,file[2][1].to_f,file[3][1].to_f,file[5][2].to_f,file[5][3].to_f,file[5][4].to_f,file[5][3].to_f+file[5][4].to_f,file[6][1].to_f,file[7][1].to_f]
end
def exe_counting2(name,th_num)
  puts "Thread Number #{th_num}"
  puts "ACCESS FILE NAME:\n"+$folder+name+"\n"
  string=name.split("distance.rst")[0].split("_") #.rstの除去と_で分割
  seed=string[-2];gen=string[-1]
  key=seed+" "+gen
  
  file=IO.readlines($folder+name).map! do |obj|
    obj.strip!.split(" ").delete_if{|el| el==""}.map!{|obj| obj.to_f}
  end
  $RESULTbox2[key]=file
end
def outputCOUNTINGbox
  File.open($folder+"../"+$CountingFileName,"w") do |f|
    $COUNTINGbox.each_with_index do |ar,i|
      if ar!=nil
        f.write("#{i+1}")
        ar.each{|value| f.write(" #{value}")}
        f.write("\n")
      end
    end
    f.write '# data format : #{generation} #{child expresivity} #{child sentence rules} #{child word rules} #{child rules} #{child to parent distance} #{WDIST} #{parent expresivity} #{parent sentence rules} #{parent word rules} #{parent rules} #{parent to child distance} #{WDIST}'+"\nNumber of detected data : #{$ITERATEnum}"
  end
end
def outputCOUNTINGbox2
  $COUNTINGbox2.each_with_index do |ar,i|
    if ar!=nil
      File.open($folder+"../"+$CountingFileName2+"_#{i+1}.dist.ave","w") do |f|
        ar.each_with_index{|value,j|
          if(value!=nil)
            f.write("#{j+1}")
            f.write(" #{value}")
            f.write("\n")
          end
        }
      end
    end
  end
end

$ITERATEnum=100
$msilm="../LEILA/msilm.exe"
$dictionary="../LEILA/data.dic"
$THREADnum=10
$RESULTbox=Hash.new
$COUNTINGbox=Array.new
$RESULTbox2=Hash.new
$COUNTINGbox2=Array.new
countProcess(ARGV)
#Example as contents of counting target file
#MSILM__1385715222_30_50.rst
##RESULT##all child to parent
#BASIC=(51,100,12,15)
#SDISTM=(0.223114)
#WDIST=(0.662862)
##RESULT##all parent to child
#BASIC=(50,100,28,24)
#SDISTM=(0.182138)
#WDIST=(0.682215)

#Example as executing "count.rb"
#ruby count.rb ~/Desktop/MSILMresult/BothBiase/
#Example as option of MSILM
# -p --path ../result/ -l -u 0.50 --analyze -g 100 --interspace-analysis 10 --term 0.8 -m 3 --interspace-logging 10 --symmetry --mutual-exclusivity
#Example as executing "exe.rb"
#ruby exe.rb ../result/ -l --analyze　-u 0.50
