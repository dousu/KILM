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
  $CountingFileName=$folder.split("/")[-1]+".acc.ave"
#  puts $CountingFileName #check

  #recognize target file name
  fileArray=Dir.entries($folder).delete_if{|obj| (not obj.include?(".mea.acc"))}
  puts "Number of files: "+fileArray.size.to_s #check

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
      th=Thread.new{num=i;value=tmp2;works[num].times{|j| exe_acc_counting(fileArray[value+j],num)}}
      threads.push(th)
      sleep 0.1#確実にSTART THREADのすぐ下にACCESS FILE NAMEがでるようにしただけ(そんな遅くないから意味ないかも.mainのほうがadvantageがでかい)
      tmp2+=works[i]
    end
#    puts tmp2
  end
  threads.each{|t| t.join}
  
  #puts $RESULTbox #check
  #puts $RESULTbox.size #check
  
  numcounter=Array.new
  
  $RESULTbox.each_pair do |key, ar|
    index=key.split(" ")[-1].to_i
    
    if $COUNTINGbox[index]==nil
      $COUNTINGbox[index]=ar.count(1).to_f
      numcounter[index]=50.0
    else
      $COUNTINGbox[index]+=ar.count(1).to_f
      numcounter[index]+=50.0
    end

  end
  $COUNTINGbox.each_with_index do |obj,i|
    if(numcounter[i]!=nil)
      $COUNTINGbox[i]/=numcounter[i]
    end
  end
  
  #output to file
  #puts $COUNTINGbox.to_s #check
  outputCOUNTINGbox

  #puts $RESULTbox.to_s #check
  
  $COUNTINGbox=nil
  $CountingFileName=$folder.split("/")[-1]+"_final.acc.ave"
  $RESULTbox.each_pair do |key, ar|
    if(key.split(" ")[-1]==$ITERATEnum.to_s)
      if $COUNTINGbox==nil
        $COUNTINGbox=ar.dup
      else
        ar.each_with_index do |el,i|
          $COUNTINGbox[i]+=el
        end
      end 
    end
  end
  $COUNTINGbox.each_with_index do |obj,i|
    if(obj!=nil)
      $COUNTINGbox[i]=obj.to_f/$ITERATEnum.to_f
    end
  end

  outputCOUNTINGbox

  $COUNTINGbox=nil
  $CountingFileName=$folder.split("/")[-1]+"_first.acc.ave"
  $RESULTbox.each_pair do |key, ar|
    if(key.split(" ")[-1]=="1")
      if $COUNTINGbox==nil
        $COUNTINGbox=ar.dup
      else
        ar.each_with_index do |el,i|
          $COUNTINGbox[i]+=el
        end
      end 
    end
  end
  $COUNTINGbox.each_with_index do |obj,i|
    if(obj!=nil)
      $COUNTINGbox[i]=obj.to_f/$ITERATEnum.to_f
    end
  end

  outputCOUNTINGbox
  
end
def exe_acc_counting(name,th_num)
  puts "Thread Number #{th_num}"
  puts "ACCESS FILE NAME:\n"+$folder+name+"\n"
  #$RESULTboxに"#{seed} #{gen}"をkeyとして[各発話が正答かどうか]の配列を突っ込む
  string=name.split(".mea.acc")[0].split("_") #.mea.accの除去と_で分割
  seed=string[-1]
  
  file=IO.readlines($folder+name).map! do |obj|
    obj.strip!.split(" ").delete_if{|el| el==""}.map!{|obj2| obj2.to_i}
  end
  file.size.times do |k|
    gen=(k+1).to_s
    key=seed+" "+gen
    $RESULTbox[key]=file[k]
  end
end
def outputCOUNTINGbox
  File.open($folder+"../"+$CountingFileName,"w") do |f|
    $COUNTINGbox.each_with_index do |ar,i|
      if ar!=nil
        f.write("#{i}")
        f.write(" #{$COUNTINGbox[i]*100.0}")
        f.write("\n")
      end
    end
  end
end

$ITERATEnum=100
$msilm="../SOURCE/msilm.exe"
$dictionary="../SOURCE/data.dic"
$THREADnum=10
$RESULTbox=Hash.new
$COUNTINGbox=Array.new
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
