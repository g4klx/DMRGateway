#!/bin/bash


source /opt/script/script.conf

exec 3>&1 4>&2
trap 'exec 2>&4 1>&3' 0 1 2 3
exec 1>/var/log/script/checkBMAPI.log 2>&1





#check BM API

check_BM_API(){
		
	echo "Repeater $call $repeater"
	
	date

	curl "https://api.brandmeister.network/v1.0/repeater/?action=profile&q=$repeater" > /var/log/script/test1.txt

	sed -n -e 's/^.*tarantool"}],//p' /var/log/script/test1.txt > /var/log/script/test2.txt

	sed 's/}],"timedSubscriptions.*//' /var/log/script/test2.txt > /var/log/script/test3.txt

	sed -n -e 's/^.*slot":1,"timeout"://p' /var/log/script/test3.txt > /var/log/script/slot1.txt



	sed -n -e 's/^.*slot":2,"timeout"://p' /var/log/script/test3.txt > /var/log/script/test4.txt

	#sed -n -e 's/^.*slot":0,"timeout"://p' /Var/log/script/test3.txt > /var/log/script/test4.txt

	
	sed 's/},{"repeaterid.*//' /var/log/script/test4.txt > /var/log/script/slot2.txt

}


check_status_slot1(){

	if [ -s "/var/log/script/slot1.txt" ]
	then
		echo "Dynamic is linked slot 1."
 		echo -n "" > /var/log/script/static-slot1.txt
	else
		echo
		#echo "Dynamic is not linked slot 1."./check
	fi
	if [ -s "/var/log/script/static-slot1.txt" ]
	then
		echo "Static is linked slot 1."
		echo
 	else
		#echo "Static is not linked slot 1."
		echo

	fi
}

check_status_slot2(){

	if [ -s "/var/log/script/slot2.txt" ]
	then
		echo "Dynamic is linked slot 2."
 		echo -n "" > /var/log/script/static-slot2.txt
	else	
		echo
		#echo "Dynamic is not linked slot 2."
	fi
	if [ -s "/var/log/script/static-slot2.txt" ]
	then
		echo "Static is linked slot 2."
 		echo
	else
		#echo "Static is not linked slot 2."
		echo

	fi
}


#check if static-slot1.txt and slot.txt are blank 

check_if_nothing_linked_slot1(){

    	if [ -s "/var/log/script/static-slot1.txt" ]
	
	then
	    	echo "Linked to Static $defaultslot1 Slot 1"
	
	elif [ -s "/var/log/script/slot1.txt" ]
        
	then
		echo "Dynamic is Linked slot 1"
	else
		echo "DMRGateway slot 1 is Unlinked" 
		echo "Linking slot 1 $defaultslot1" && sleep $minrelinktimerslot1; echo "DynTG1,$defaultslot1" > /dev/udp/127.0.0.1/3769
		echo -n "Linked to $defaultslot1" > /var/log/script/static-slot1.txt

		
	fi 
	
}



#check if static-slot2.txt and slot.txt are blank 

check_if_nothing_linked_slot2(){

    	if [ -s "/var/log/script/static-slot2.txt" ]
	
	then
	    	echo "Linked to Static $defaultslot2 Slot 2"
	
	elif [ -s "/var/log/script/slot2.txt" ]
        
	then
		echo "Dynamic is Linked slot 2"
	else
		echo "DMRGateway slot 2 is Unlinked" 
		echo "Linking slot 2 $defaultslot2" && sleep $minrelinktimerslot2; echo "DynTG2,$defaultslot2" > /dev/udp/127.0.0.1/3769
		echo -n "Linked to $defaultslot2" > /var/log/script/static-slot2.txt
	fi 
	
}



while :
do
check_BM_API
echo
$checkslot1
$checkslot2
$slot1notlinked
$slot2notlinked
echo
sleep $slowdown
done
