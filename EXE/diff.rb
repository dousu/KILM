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
  
  $RESULT1=IO.readlines(filelist[0]).map! do |obj|
    #puts "Z"+obj.to_s+"Z"
    if obj.strip!
      obj.split(" ").delete_if{|el| el==""}
    else
      obj
    end
  end
  $RESULT1.slice!($RESULT1.size-2...$RESULT1.size)

  #puts $RESULT1.to_s

  $RESULT2=IO.readlines(filelist[1]).map! do |obj|
    #puts "Z"+obj.to_s+"Z"
    if obj.strip!
      obj.split(" ").delete_if{|el| el==""}
    else
      obj
    end
  end
  $RESULT2.slice!($RESULT2.size-2...$RESULT2.size)
  
  if($RESULT1.size!=$RESULT2.size)
    puts filelist[0]+" has" +$RESULT1+" rows" 
    exit(-1)
  end
  
  average=Array.new($RESULT1[0].size-1,0.0)
  $RESULT1.each_with_index do |obj1,ind1|
    obj1.each_with_index{|obj2,ind2| average[ind2-1]+=(obj2.to_f-$RESULT2[ind1][ind2].to_f)/$RESULT1.size.to_f if ind2!=0}
  end
  
  File.open($BASE_DIR+"diff.dff","w") do |f|
    $RESULT1.each_with_index do |obj,ind|
      f.write "#{ind+1}"
      obj.each_with_index{|obj2,ind2| f.write(" #{obj2.to_f-$RESULT2[ind][ind2].to_f}") if ind2!=0}
      f.write("\n")
    end
    f.write("Av.")
    average.each{|obj2| f.write " #{obj2}"}
  end
  
end

$MAX_GEN=300
$MAX_EXP=100
$MAX_NUM=100
$MAX_DIST=1
$BASE_DIR="/home/hiroki/Desktop/"
$PSTSCRPT_POINT=20
$PREFIX=ARGV[0]

if ARGV.size!=3
  puts "Number of argment is wrong. It should be 3 argment"
  exit(-1)
end

graphILM(ARGV[1..ARGV.size])
