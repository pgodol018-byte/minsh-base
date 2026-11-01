# Submission

## Members
### Member 1
- Name: SPYRIDON KANIDIS
- Academic ID:IIS23200

### Member 2
- Name: GKONTOLIAS PANAGIWTHS
- Academic ID:IIS25110

## Σύντομη περιγραφή υλοποίησης
STRUCT-JOB : ΔΙΑΤΗΡΕΙ ΤΟ PID KAI ΤΗΝ ΚΑΤΑΣΤΑΣΗ (active) καθε διεργασίας
STATIC STRUCT JOB JOBS[MAX_JOBS]: ΠΙΝΑΚΑΣ ΣΤΑΘΕΡΟΥ ΜΕΓΕΘΟΥΣ ΓΙΑ ΤΗΝ ΔΙΑΧΕΙΡΙΣΗ JOBS 
STRUCT PIPELINE KAI STRUCT COMMAND : ΔΟΜΕΣ ΤΟΥ ΠΑΡΣΕΡ



print_prompt : ΧΡΗΣΙΘΜΟΠΟΙΕΙ ΤΗΝ ΣΥΝΑΡΤΗΣΗ "Getcwd()" για να διαβαζει δυναμικα το μονοματι
execute_pipeline():κεντρικη συναρτηση εκτελεσης ----
Διαχειριζεται τις built-in εντολες (cd,exit)
Δημιουργει pipes
Εκτελλει fork()
Διαχειριζεται τα σηματα ("SignInt,SigChild) και μπλοκαρει το Sigchild 
handle_sigchld() kai rea_bg_jobs()


Δοκιμές εσωτερικών εντολών (cd , exit)
cd/tmp
cd παει στο Home
exit !!!!!!

ΕΙΣΟΔΟΙ ΕΞΟΔΟΙ
ECHO"MPLAMPLA">text.txt
cat<test.txt
echo "MpleMple">>test.txt

ls-l|grep
cat/etc/passwd |grep-v"nobody"|wc-l

sleep 5 &
